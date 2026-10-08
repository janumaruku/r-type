> **Reference document.** This is the journey written while building the ECS of the Epitech bootstrap (`B-CPP-500_rtype_bootstrap.pdf`), copied here as background for the `engine_ecs` design. It explains *why* the ECS works the way it does and lists the traps and tests of each step. Where it differs from [`modules/engine.md`](../modules/engine.md) § ECS (namespaces, build system, SFML demo, `std::any`), **the spec wins**: R-Type implements the journey's step 9 design together with its steps 10, 11 and 12 ([ADR 0005](../decisions/0005-ecs-design.md)).

# Building an ECS in C++: from the R-Type bootstrap to a sparse-set ECS

This document retraces, step by step, how the ECS in this repository was built:
what each piece does, **why** it is designed that way, which C++ tools it uses,
which traps were found along the way, and what was measured. It is written so
that you can rebuild everything from an empty directory by following it in
order.

- Steps 1 to 7 follow the Epitech bootstrap (`B-CPP-500_rtype_bootstrap.pdf`),
  with the places where we deliberately departed from it explained.
- Steps 8 and 9 go beyond it: sparse sets, then a registry and views built on them.
- The last part lists the next improvements (steps 10+), with enough detail to
  implement them.

Every step ends with **"Check yourself"**: the tests that prove the step works.
Write them before moving on. Several bugs in this project were only caught
that way.

Steps that claim a performance gain also have a **"Benchmark"** section: what
to measure to prove the claim, the benchmark code (Google Benchmark, see
[Benchmarking with Google Benchmark](#benchmarking-with-google-benchmark)), the
numbers we got, and what they show. Tests show the code is correct; benchmarks
show a design choice was worth it.

---

## Table of contents

- [Setup: tools, layout, conventions](#setup-tools-layout-conventions)
- [Benchmarking with Google Benchmark](#benchmarking-with-google-benchmark)
- [Step 0: What an ECS is](#step-0-what-an-ecs-is)
- [Step 1: The entity](#step-1-the-entity)
- [Step 2: SparseArray, a component column](#step-2-sparsearray-a-component-column)
- [Interlude: project structure, `.tpp` files and CMake](#interlude-project-structure-tpp-files-and-cmake)
- [Step 3: The registry, storing different types together](#step-3-the-registry-storing-different-types-together)
- [Step 4: Entity lifecycle](#step-4-entity-lifecycle)
- [Step 5: Using the ECS, the SFML demo](#step-5-using-the-ecs-the-sfml-demo)
- [Step 6: Systems inside the registry](#step-6-systems-inside-the-registry)
- [Step 7: The zipper](#step-7-the-zipper)
- [Step 8: Sparse sets and the memory footprint](#step-8-sparse-sets-and-the-memory-footprint)
- [Step 9: The registry on sparse sets, and views](#step-9-the-registry-on-sparse-sets-and-views)
- [Testing strategy](#testing-strategy)
- [Current rules and limitations](#current-rules-and-limitations)
- [Further improvements (steps 10+)](#further-improvements-steps-10)
- [Appendix: file map and C++ feature index](#appendix-file-map-and-c-feature-index)

---

## Setup: tools, layout, conventions

### Tools

| Tool | Version used | Notes |
|---|---|---|
| Compilers | GCC 13, Clang 18 | Always test with **both**: they differ in warnings and in optimization (see step 7). |
| Standard | C++20 | Needed for concepts, `std::span`, `std::remove_cvref_t`, `<ranges>`, template lambdas. |
| CMake | 3.28 | `FetchContent` with `SYSTEM` / `EXCLUDE_FROM_ALL`. |
| GoogleTest | fetched by CMake | Same pinned version as the zappy project. |
| SFML | **3.1.0**, built from source | Only for the demo (step 5). Ubuntu's `libsfml-dev` is 2.6, whose API is incompatible. |

**Installing SFML 3.1.0** (once per machine):

```bash
sudo apt install build-essential cmake git pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev \
    libfreetype-dev libharfbuzz-dev libvorbis-dev libflac-dev
cd /tmp
git clone --depth 1 --branch 3.1.0 https://github.com/SFML/SFML.git sfml-3.1.0
cd sfml-3.1.0
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSFML_BUILD_NETWORK=OFF
cmake --build build -j"$(nproc)"
sudo cmake --install build      # installs into /usr/local
sudo ldconfig
```

`SFML_BUILD_NETWORK=OFF`: SFML 3.1's network module needs MbedTLS 3+, but
Ubuntu 24.04 ships 2.28. We don't need SFML's network module anyway.

### Repository layout

The layout mirrors the zappy project:

```
CMakeLists.txt              root: warning_options, options, subdirectories
libs/
  CMakeLists.txt
  ECS/
    CMakeLists.txt          static library `ecs`
    Registry.cpp            the only non-template code
    include/                *.hpp (declarations) + *.tpp (template definitions)
src/
  CMakeLists.txt
  Bootstrap/                the SFML demo (step 5)
    CMakeLists.txt
    main.cpp
    Systems.cpp
    include/Components.hpp
    include/Systems.hpp
tests/
  CMakeLists.txt            GoogleTest via FetchContent, target `unit_tests`
  ECS/*Tests.cpp
benchmarks/
  CMakeLists.txt            target `storage_benchmark`
  StorageBenchmark.cpp
docs/
  ECS_JOURNEY.md            this document
```

### Code conventions

- Epitech header comment at the top of every file.
- `PascalCase` classes, `camelCase` methods, `_member` for private data.
- **Exceptions, on purpose:** names that generic code looks for stay in the
  standard style: `begin()`, `end()`, `size()`, and the type aliases
  `value_type`, `iterator`, `size_type`, `reference`... A range-for only finds
  `begin`/`end`, and templates such as `<algorithm>` read the aliases by name.
- Every target links `warning_options` (`-Wall -Wextra -Werror`).

### Build commands

```bash
# Everything (demo + tests)
cmake -S . -B build -DBUILD_TESTING=ON && cmake --build build -j && ./unit_tests

# ECS + tests only, no SFML needed (CI machines, no display)
cmake -S . -B build -DBUILD_BOOTSTRAP=OFF -DBUILD_TESTING=ON

# With sanitizers (do this regularly)
cmake -S . -B build-asan -DBUILD_TESTING=ON \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"

# Benchmarks (Release only: Debug numbers are meaningless)
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build build-release --target storage_benchmark
./build-release/benchmarks/storage_benchmark
```

Binaries (`bootstrap`, `unit_tests`) are written to the repository root, as in
zappy, and are listed in `.gitignore`.

---

## Benchmarking with Google Benchmark

### Why a library instead of a stopwatch

During the journey, the first measurements were hand-rolled
(`benchmarks/StorageBenchmark.cpp`: `std::chrono::steady_clock`, best of N
runs, `noinline` functions, a `volatile` sink). That answered yes/no questions,
but it has real limits:

- **No statistics:** "best of 20" hides variance. A manual-loop time once moved
  from 4.3 to 5.3 ms between two runs, and nothing in the output said so.
- **Fixed repetitions:** a 0.001 ms case gets the same number of runs as a 10 ms
  case, so short cases are close to timer resolution.
- **No warnings** about CPU frequency scaling, machine load or Debug builds.
- **No comparison tool:** comparing two versions meant reading two tables by eye.

**Google Benchmark** (v1.9.5, `benchmark::benchmark` in CMake) solves all four:
it picks the iteration count automatically until the measurement is stable,
reports mean, median, standard deviation and coefficient of variation over
repetitions, prints machine warnings, writes JSON, and ships a comparison tool
with a statistical test (`compare.py`). It's the sibling of GoogleTest, set up
the same way.

### Setup

`benchmarks/CMakeLists.txt`:

```cmake
include(FetchContent)

# We only want the library: no tests, no install rules, no -Werror on its code.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.5
        GIT_SHALLOW ON
        SYSTEM
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(ecs_benchmarks
    StorageBenchmarks.cpp       # steps 2, 8
    RegistryBenchmarks.cpp      # steps 3, 4, 6
    IterationBenchmarks.cpp     # steps 7, 9
)
target_include_directories(ecs_benchmarks PRIVATE .)
target_link_libraries(ecs_benchmarks PRIVATE ecs benchmark::benchmark_main)
```

`benchmark::benchmark_main` provides `main()`. Each step below adds its
benchmarks to one of those files.

> **State of the repository:** at the time of writing, `benchmarks/` still
> contains the hand-rolled `StorageBenchmark.cpp` (target `storage_benchmark`)
> used during steps 7–9. The Google Benchmark target described here is what to
> build when following this document; every benchmark shown in the steps was
> compiled and run against Google Benchmark v1.9.5 and this ECS. Shared helpers go in
`benchmarks/BenchmarkHelpers.hpp`:

```cpp
inline constexpr std::size_t ENTITY_COUNT = 1'000'000;

struct Position { float x; float y; };
struct Small { float value; float pad; };       // 8 bytes
struct Big { float value; float pad[15]; };     // 64 bytes
template <std::size_t N> struct Tag {};         // as many distinct types as needed

// A random subset of ids (fixed seed), in increasing order, as entities
// usually get components in the order they spawn.
inline std::vector<std::size_t> pickEntities(double density)
{
    std::mt19937 rng(42);
    std::bernoulli_distribution owns(density);
    std::vector<std::size_t> ids;

    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        if (owns(rng))
            ids.push_back(e);
    return ids;
}

// Benchmarks take the density in per mille as their argument: /1 = 0.1%, /1000 = 100%.
inline double densityOf(const benchmark::State &state)
{
    return static_cast<double>(state.range(0)) / 1000.0;
}
```

### Anatomy of a benchmark

```cpp
template <class T>                                       // one benchmark per component type
void BM_Join_View(benchmark::State &state)
{
    // Setup: runs once, NOT timed.
    ecs::SparseSet<Position> positions;
    ecs::SparseSet<T> others;
    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        positions.insertAt(e, Position{1.f, 0.f});
    for (std::size_t e : pickEntities(densityOf(state)))
        others.insertAt(e, T{1.f, {}});

    for (auto _ : state) {                               // ONLY this loop is timed
        float sum = 0.f;
        for (auto &&[pos, other] : ecs::View(positions, others))
            sum += pos.x * other.value;
        benchmark::DoNotOptimize(sum);                   // the work can't be optimized away
    }
    state.counters["matches"] = static_cast<double>(others.size());   // extra column
}
BENCHMARK_TEMPLATE(BM_Join_View, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000)     // densities, read with state.range(0)
    ->Unit(benchmark::kMillisecond);
```

- **`for (auto _ : state)`**: the library decides how many iterations to run.
  Setup goes before the loop.
- **`benchmark::DoNotOptimize(x)`**: forces `x` to be computed (it replaces the
  `volatile` sink). `benchmark::ClobberMemory()` forces pending memory writes.
- **`state.counters["name"]`**: any number as an extra column (memory in MB,
  matches...). `state.SetItemsProcessed(n)` adds an `items_per_second` column.
- **`->Arg`, `->Range(lo, hi)` with `->RangeMultiplier(k)`**: parameter sweeps.
  **`BENCHMARK_TEMPLATE`**: one benchmark per type.
- **`->Complexity()`** with `state.SetComplexityN(n)`: fits O(1)/O(N)/O(N²)...
  to the results.
- **`state.PauseTiming()` / `ResumeTiming()`**: excludes per-iteration setup, but
  costs about a microsecond each call. Use it only when the timed part is much
  longer than that.

### Running

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build build-release --target ecs_benchmarks

B=./build-release/benchmarks/ecs_benchmarks
$B                                                     # everything
$B --benchmark_filter='BM_Join_.*<Small>'              # regex on names
$B --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
$B --benchmark_min_time=0.5s                           # or 100x for a fixed iteration count
$B --benchmark_out=results.json --benchmark_out_format=json
```

### Reading the output

Excerpt of a real run of this project's benchmarks:

```
Run on (4 X 2900 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L2 Unified 256 KiB (x2)
  L3 Unified 3072 KiB (x1)
Load Average: 5.36, 5.20, 3.41
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------------------------
Benchmark                                      Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------
BM_Registry_GetComponents                   33.8 ns         33.8 ns      8678726
BM_Registry_RunSystems/10                    703 ns          703 ns       398287 items_per_second=14.2319M/s
BM_Join_SparseSetManual<Small>/10          0.119 ms        0.119 ms         2111 matches=9.976k
BM_Erase_SwapAndPop/16384                  77475 ns        77273 ns         3622
BM_Erase_SwapAndPop_BigO                    4.72 N          4.71 N
```

- **Header:** the cache sizes explain where performance changes (8 MB of
  `_sparse` doesn't fit in a 3 MB L3). **Load Average** above the core count (here
  5.36 on 4 cores) means other programs compete for the CPU: **don't trust
  that run**. Close them and rerun with repetitions.
- **Time** is wall-clock time per iteration, **CPU** is CPU time; they differ
  when the process waits. **Iterations** is how many times the loop ran.
- With repetitions, each case gets `_mean`, `_median`, `_stddev` and `_cv`
  (coefficient of variation = stddev / mean). **Read the median, and trust it
  when cv is small** (a few %). A 3% "speedup" with a 5% cv is noise.

### Comparing: proving that a change helped

`compare.py` lives in the Google Benchmark sources
(`_deps/googlebenchmark-src/tools/compare.py` in the build directory). It needs
`pip install numpy scipy`.

```bash
# Two implementations in the same run (e.g. view vs hand-written join):
compare.py filters ./ecs_benchmarks 'BM_Join_SparseSetManual' 'BM_Join_View'

# Before / after a change: one JSON per version, then compare them
$OLD --benchmark_repetitions=10 --benchmark_out=before.json --benchmark_out_format=json
$NEW --benchmark_repetitions=10 --benchmark_out=after.json  --benchmark_out_format=json
compare.py benchmarks before.json after.json
```

For each benchmark it prints the relative time difference (`-0.25` = 25%
faster) and, with repetitions, a **Mann-Whitney U-test** p-value telling whether
the difference is statistically meaningful or noise. To get the "before"
binary, build the previous commit in another directory (`git worktree add
../ecs-before HEAD~1`).

### Rules for trustworthy numbers

1. **Release builds only, with the flags you ship.** Never benchmark Debug or
   sanitizer builds. CMake's `Release` is `-O3 -DNDEBUG`; `RelWithDebInfo` is
   `-O2 -g`. The optimization level can change conclusions: in step 9, the View
   was about 1.1× the hand-written join with GCC `-O3` but about 2× with `-O2`.
   When a result matters, measure both.
2. **A quiet machine:** close browsers and IDEs, and check the load average in
   the header. Optionally pin the process to one core (`taskset -c 2 $B`) and set
   the CPU governor to `performance`.
3. **Repetitions + median + cv** rather than a single run.
4. **Both compilers:** in step 7, the same code was 2× slower with GCC than
   with Clang.
5. **Compare against a baseline written by hand:** "the view takes 3 ms" means
   nothing alone; "the view takes 1.1× the hand-written loop" does.
6. **Benchmark what the claim is about.** Counting copies is a job for a test
   (step 2): an insert benchmark is dominated by memory allocation and can't see
   one copy more or less.

---

## Step 0: What an ECS is

### The problem with inheritance

The usual OOP design for a shoot'em up:

```
GameObject
├── Movable
│   ├── Player
│   ├── Enemy
│   │   └── ShootingEnemy
│   └── Bullet
└── Static
    └── Wall
```

Then the designers ask for a turret that shoots but doesn't move, a wall that
moves, a power-up that gives the player homing bullets for 5 seconds... Every new
combination fights the tree: diamond inheritance, giant base classes, or
changing an object's class at runtime. The root issue: **inheritance mixes what
an object *is* with what it *can do***, and fixes both at compile time.

### Entity, Component, System

| Piece | What it is | Example |
|---|---|---|
| **Entity** | Only an identifier (a number). No data, no behavior. | `42` |
| **Component** | Plain data, no logic. | `Position{x, y}`, `Velocity{vx, vy}`, `Health{hp}` |
| **System** | A function that works on every entity having a given set of components. | `movement(Position, Velocity)` |

An entity's behavior is the set of components attached to it:

- Player = Position + Velocity + Sprite + Controllable + Health
- Turret = Position + Sprite + Shooter + Health (no Velocity, so the movement system ignores it)
- Homing power-up = **add** a `Homing` component, **remove** it 5 seconds later.

Nothing changes type. Behavior is composed, and it can change at runtime.

### The mental model: a database table

```
 entity │ Position │ Velocity │ Sprite │ Controllable │ Health
────────┼──────────┼──────────┼────────┼──────────────┼────────
   0    │  (10,5)  │  (1,0)   │ ship   │      ✓       │   3
   1    │  (50,5)  │    —     │ wall   │      —       │   —
   2    │  (80,9)  │  (-2,0)  │ alien  │      —       │   1
   3    │    —     │    —     │   —    │      —       │   —    ← free id
```

- An **entity** is a row number.
- A **component type** is a column.
- A **system** is a query: *"for every row where Position AND Velocity are
  present, do pos += vel"*.

That is why the bootstrap says "entities don't need to exist": a row number is
all an entity is.

### The smallest possible ECS

Before any template, this is already an ECS:

```cpp
struct Position { float x, y; };
struct Velocity { float vx, vy; };

struct World {
    std::vector<std::optional<Position>> positions;   // one column per component
    std::vector<std::optional<Velocity>> velocities;

    std::size_t spawn()                               // an entity = a new row
    {
        positions.emplace_back();
        velocities.emplace_back();
        return positions.size() - 1;
    }
};

void movementSystem(World &w)                        // a system = a query
{
    for (std::size_t e = 0; e < w.positions.size(); ++e)
        if (w.positions[e] && w.velocities[e]) {
            w.positions[e]->x += w.velocities[e]->vx;
            w.positions[e]->y += w.velocities[e]->vy;
        }
}
```

Every step that follows removes one limitation of this toy:

| Limitation | Fixed by | Step |
|---|---|---|
| A raw `size_t` can be passed where an entity is expected | strong `Entity` type | 1 |
| `vector<optional<T>>` written by hand for each type | generic `SparseArray<T>` | 2 |
| `World` hard-codes every component type | storage looked up by type (`type_index` + type erasure) | 3 |
| Removing an entity means touching every column by name | type-erased erase functions | 4 |
| Systems fetch their own columns | `addSystem<Components...>(f)` | 6 |
| The `if (a && b)` loop is repeated everywhere | zipper / view iterators | 7, 9 |
| Every row is visited, even without the components | sparse sets | 8, 9 |
| A recycled id can be mistaken for the dead entity | generational ids | 11 (future) |

### Exercise from this step

A bullet needs Position, Velocity and a Sprite (handled by the movement and draw
systems), plus Hitbox, Damage and Team for collisions, and a
`DestroyOffscreen` **tag component** (an empty struct) handled by a system that
kills entities leaving the screen. With no class, "delete me" logic lives in a
system, so **systems must be able to create and destroy entities**. That's
why every system later receives the registry.

---

## Step 1: The entity

**Bootstrap step 0.** Files: `libs/ECS/include/Entity.hpp`, `tests/ECS/EntityTests.cpp`.

### Why a strong type

```cpp
void killEntity(std::size_t e);
killEntity(playerHealth);       // compiles, kills the wrong thing
```

Wrapping the number in a class gives it a meaning the compiler checks.

| Direction | Rule | Reason |
|---|---|---|
| `Entity` → `size_t` | **implicit** | Used as an array index all the time: `positions[e]`. |
| `size_t` → `Entity` | **explicit** only | `Entity e = 3;` must not compile: entities come from the code that creates them. |

### The code (first version)

```cpp
namespace ecs {

class Entity {
public:
    using id_type = std::size_t;

    constexpr explicit Entity(id_type id) noexcept : _id(id) {}
    constexpr operator id_type() const noexcept { return _id; }

private:
    id_type _id;
};

}
```

- **`explicit`:** a constructor with one argument is also an implicit
  conversion unless marked `explicit`. With it, `Entity{3}` and
  `static_cast<Entity>(3)` work, but `Entity e = 3;` and `f(3)` (where `f`
  takes an `Entity`) do not.
- **`operator id_type()`** is a *conversion operator*. Without `explicit`, it
  is implicit, so `vector[e]` works.
- **`const`** (converting doesn't modify), **`noexcept`** (can't fail),
  **`constexpr`** (usable at compile time): free, so always add them.
- **No default constructor:** declaring any constructor removes the implicit
  default one. `Entity e;` has no meaning. (Generational ids, step 11, will
  introduce a "null" entity.)
- **Known gap:** `e == 3` compiles, because `e` converts to `size_t`. That's
  the price of the implicit conversion the bootstrap asks for.

**Later (step 4):** the constructor becomes `private`, with `friend class
Registry;`, so only the registry can create entities.

### Check yourself

What must *not* compile can't be a runtime test. Use type traits:

```cpp
static_assert(!std::is_convertible_v<std::size_t, Entity>);     // Entity e = 3; rejected
static_assert(std::is_convertible_v<Entity, std::size_t>);      // size_t i = e; accepted
static_assert(!std::is_default_constructible_v<Entity>);
static_assert(sizeof(Entity) == sizeof(std::size_t));           // zero-cost wrapper
```

Before step 4, add `static_assert(std::is_constructible_v<Entity, std::size_t>)`.
After step 4, it becomes `!std::is_constructible_v<...>`, because type traits
check from outside the class, where the constructor is private.

---

## Step 2: SparseArray, a component column

**Bootstrap step 1.** Files: `SparseArray.hpp` / `SparseArray.tpp`, `tests/ECS/SparseArrayTests.cpp`.

### The data structure

One column of the table. The component of entity N is at index N; missing
components are empty optionals ("holes"):

```
entity:    0        1       2        3
_data:   [(10,5)] [ --- ] [(80,9)] [ --- ]     std::vector<std::optional<Position>>
```

`std::optional<T>` is a `T` plus a `bool`, stored **inline** (no heap
allocation). Test it with `if (opt)`, read it with `*opt` or `opt->x`. Cost:
`sizeof(optional<Position>)` is 12 bytes for an 8-byte `Position`, and **every
hole costs that too**. Keep this in mind: it's what step 8 fixes.

### The interface

```cpp
template <typename Component>
class SparseArray {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type &;
    using const_reference_type = const value_type &;
    using container_t = std::vector<value_type>;
    using size_type = container_t::size_type;
    using iterator = container_t::iterator;
    using const_iterator = container_t::const_iterator;

    static constexpr size_type npos = static_cast<size_type>(-1);

    SparseArray() = default;
    SparseArray(const SparseArray &) = default;
    SparseArray(SparseArray &&) noexcept = default;
    ~SparseArray() = default;
    SparseArray &operator=(const SparseArray &) = default;
    SparseArray &operator=(SparseArray &&) noexcept = default;

    reference_type operator[](size_type idx);
    const_reference_type operator[](size_type idx) const;

    iterator begin() noexcept;              // + const begin(), cbegin(), end()...
    [[nodiscard]] size_type size() const noexcept;

    reference_type insertAt(size_type pos, const Component &component);
    reference_type insertAt(size_type pos, Component &&component);
    template <class... Params>
    reference_type emplaceAt(size_type pos, Params &&...params);
    void erase(size_type pos);
    [[nodiscard]] size_type getIndex(const value_type &value) const noexcept;

private:
    container_t _data;
};
```

### Design points

**Type aliases.** They follow the standard container conventions because
generic code relies on them. The zipper (step 7) writes `typename
Container::iterator` without knowing what the container is. Before C++20,
`typename` was required in front of `container_t::size_type` (a *dependent*
name); C++20 allows dropping it in an alias declaration.

**Rule of zero.** The only member is a `std::vector`, which already copies,
moves and destroys itself correctly. Writing these functions by hand would only
add bugs. They're spelled out as `= default` only to match the bootstrap.
**`noexcept` on the moves matters:** `std::vector<SparseArray<X>>` only *moves*
its elements when growing if the move constructor is `noexcept`, otherwise it
copies them (`std::move_if_noexcept`).

**`operator[]` is unchecked**, like `vector::operator[]`, with an `assert` that
disappears in Release (`-DNDEBUG`). It returns the **optional**, because the
caller must check for a hole.

**`insertAt`: two overloads, for value categories.** `insertAt(3, myPos)`
receives an *lvalue* (a named object), so it copies. `insertAt(3,
Position{1, 2})` receives an *rvalue* (a temporary), so it can move (that
matters for components owning a string or a vector).

**The aliasing trap.**

```cpp
arr.insertAt(1000, *arr[0]);   // the source lives inside arr
```

`resize` may **reallocate**: allocate a new buffer, move everything, free the
old one. `component` then refers to freed memory. AddressSanitizer reports a
heap-use-after-free on the naive version. The fix is to take a copy *before*
growing:

```cpp
template <typename Component>
auto SparseArray<Component>::insertAt(size_type pos, const Component &component)
    -> reference_type
{
    if (pos < _data.size()) {
        _data[pos] = component;
        return _data[pos];
    }
    value_type tmp{component};    // copy while the reference is still valid
    _data.resize(pos + 1);        // may reallocate
    _data[pos] = std::move(tmp);  // only use tmp
    return _data[pos];
}
```

Whenever you write a container that can reallocate, ask: *can the argument
point into me?* (`std::vector::push_back` handles it internally.)

**`emplaceAt`: variadic templates and perfect forwarding.**

```cpp
template <typename Component>
template <class... Params>
auto SparseArray<Component>::emplaceAt(size_type pos, Params &&...params) -> reference_type
{
    if (pos < _data.size()) {
        _data[pos].emplace(std::forward<Params>(params)...);
        return _data[pos];
    }
    value_type tmp{std::in_place, std::forward<Params>(params)...};
    _data.resize(pos + 1);
    _data[pos] = std::move(tmp);
    return _data[pos];
}
```

- `class... Params` is a **template parameter pack**: any number of types.
- `Params &&...params` are **forwarding references**. When `T` is *deduced*,
  `T&&` means "whatever was passed, lvalue or rvalue". Passing an lvalue `x` of
  type `X` deduces `Params = X&`, and reference collapsing turns `X& &&` into
  `X&`. Passing an rvalue deduces `Params = X`, so the parameter is `X&&`.
- Inside the function, `params` has a name, so it is an lvalue.
  **`std::forward<Params>(params)`** restores the original category. The
  trailing `...` expands the pack:
  `std::forward<P1>(p1), std::forward<P2>(p2), ...`.
- `optional::emplace` destroys the current value (if any) and constructs the new
  one in place. The bootstrap suggests doing it by hand with
  `std::allocator_traits::destroy` + `construct`, which is the same operation;
  `emplace` already does it correctly.
- `std::in_place` tells `optional`'s constructor that the following arguments
  are for building the `T` inside it.

**`erase` leaves a hole, never shifts:** the index *is* the entity id. Shifting
would give entity 5's data to entity 4. The vector isn't shrunk either, because
entities come and go every frame.

**`getIndex` finds a slot by address, not by value:** two bullets can have equal
velocities, and a component may not even have `operator==`. All slots live in
one buffer, so the index is `ptr - data()`:
- `std::addressof(value)` instead of `&value`: a type may overload `operator&`.
- `std::less<const value_type *>` instead of `<`: comparing pointers into
  different arrays with `<` is unspecified; `std::less` guarantees a total order.
- Returns `npos` when the optional doesn't belong to the array.

**Iteration visits every slot, holes included**, so a system costs O(highest
entity id), not O(entities with the component). That limitation remains until
step 8.

### Benchmark: iteration cost follows the highest id, not the component count

**Claim to verify:** with a `SparseArray`, fewer components don't make
iteration cheaper. This is the baseline that step 8 improves on.

`StorageBenchmarks.cpp`:

```cpp
template <class T>
void BM_SparseArray_Iterate(benchmark::State &state)
{
    ecs::SparseArray<T> array;
    for (std::size_t e : pickEntities(densityOf(state)))
        array.insertAt(e, T{1.f, {}});

    for (auto _ : state) {
        float sum = 0.f;
        for (const auto &slot : array)
            if (slot)
                sum += slot->value;
        benchmark::DoNotOptimize(sum);
    }
    state.counters["MB"] = static_cast<double>(array.size() * sizeof(std::optional<T>)) / 1e6;
}
BENCHMARK_TEMPLATE(BM_SparseArray_Iterate, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000)->Unit(benchmark::kMillisecond);
```

**Result** (GCC 13, `-O2`, 1M ids):

| density | 0.1% | 1% | 10% | 50% | 100% |
|---|---|---|---|---|---|
| time | 1.24 ms | 1.27 ms | 2.00 ms | **5.17 ms** | 1.27 ms |
| memory | 12 MB | 12 MB | 12 MB | 12 MB | 12 MB |

1000 components cost as much as 1,000,000. Memory is constant, and **50% is
the slowest case**: the `if (slot)` becomes unpredictable (explained in step 8).

**Not benchmarked: copy vs move in `insertAt`.** We tried: insert-copy,
insert-move and emplace of a 64-character string component all came out within
2% of each other, because every version allocates a string and the allocation
dominates. One copy more or less is invisible in that noise. The test that
**counts** copies and moves is the right tool for that claim (rule 6 of the
benchmarking section).

### Check yourself

- Inserting at 3 into an empty array gives size 4 with holes at 0–2.
- Inserting again at the same index replaces the value.
- `erase` leaves a hole and doesn't move the others; `erase` out of range is a no-op.
- `emplaceAt` on an existing slot performs **0 copies and 0 moves**. Use a
  component type that counts its copies and moves in static counters.
- `getIndex` distinguishes two slots holding equal values, and returns `npos` for
  a foreign optional.
- `arr.insertAt(1000, *arr[0])` with a long `std::string`, under ASan.

---

## Interlude: project structure, `.tpp` files and CMake

### Why templates live in headers

A template is a recipe, not code. `SparseArray<Position>::insertAt` is only
generated in the file that uses it with `Position`, and that file needs the
**full definition**. Definitions in a `.cpp` file give "undefined reference"
errors at link time. The `.tpp` convention keeps definitions in a separate file
anyway: the `.hpp` ends with `#include "SparseArray.tpp"`.

### Defining members outside the class

```cpp
// The return type before the class name is outside the class scope:
template <typename Component>
typename SparseArray<Component>::reference_type
SparseArray<Component>::insertAt(size_type pos, const Component &component);

// Trailing return type: after `SparseArray<Component>::`, the class scope
// applies, so aliases need no prefix:
template <typename Component>
auto SparseArray<Component>::insertAt(size_type pos, const Component &component)
    -> reference_type;
```

A member template of a class template needs **two** template headers, the
class's first:

```cpp
template <typename Component>
template <class... Params>
auto SparseArray<Component>::emplaceAt(size_type pos, Params &&...params) -> reference_type
```

### CMake

- Root `CMakeLists.txt`: `warning_options` (an `INTERFACE` library carrying
  `-Wall -Wextra -Werror`), `add_subdirectory(libs)`, and the options
  `BUILD_BOOTSTRAP` (default ON), `BUILD_TESTING` (OFF), `BUILD_BENCHMARKS` (OFF).
- While everything is templates, `ecs` is an **`INTERFACE` library**: it
  compiles nothing and only passes its include path and flags to whoever links
  it. Once `Registry.cpp` exists (step 4), it becomes **`STATIC`** with
  `PUBLIC` include directories.
- `tests/CMakeLists.txt` fetches GoogleTest with `FetchContent` and builds one
  `unit_tests` executable linked to `ecs` and `GTest::gtest_main`.

---

## Step 3: The registry, storing different types together

**Bootstrap steps 2.0–2.2.** Files: `Registry.hpp` / `Registry.tpp`, `tests/ECS/RegistryTests.cpp`.

### The problem

We want **one object** owning a `SparseArray<Position>`, a
`SparseArray<Velocity>`, and so on, and returning the right one from the type
alone:

```cpp
auto &positions = registry.getComponents<Position>();
```

`std::vector<SparseArray<???>>` can't work: each template instantiation is an
**unrelated type**, and a container holds one type. Two sub-problems:

1. **The key:** turning a *type* into a *value*.
2. **The value:** storing objects of different types in one container.

### The key: `std::type_index`

| Approach | Problem |
|---|---|
| Register types in a fixed order, index a vector | Everyone must agree on the order. One mistake and Velocity data is read as Position. |
| Hand-written ids per component | Collisions, especially with types from plugins. |
| `std::string` names | Collisions across namespaces, typos the compiler can't check. |
| **`std::type_index`** | Unique per type, generated by the compiler. |

- `typeid(Position)` returns `const std::type_info &`, a record the
  compiler generates once per type (RTTI). For a *type* (not a polymorphic
  expression), it's resolved at compile time.
- `std::type_info` is not copyable, so it can't be a map key.
- `std::type_index` wraps a pointer to it and adds `==`, `<` and `std::hash`.

Container: `std::unordered_map` (O(1) average, no ordering needed).
Cost to know: libstdc++ hashes a `type_index` by hashing the **mangled type
name**, a string. That's fine at one lookup per system per frame, but it's one of
the things step 10 removes.

### The value: type erasure

**Type erasure** = storing an object while forgetting its static type, but
keeping enough to use it or get it back safely.

- **`void *`:** no safety (a wrong cast compiles), and **no destruction**:
  `delete` on a `void *` can't know which destructor to run.
- **A common base class** `IComponentArray` with a virtual destructor, and
  `static_cast` back to the real type (safe, since the key was computed from it).
  Many real ECSs do this. The bootstrap avoids inheritance.
- **`std::any`** (C++17), the bootstrap's choice. `std::any_cast<T &>(a)` checks
  the type and throws `std::bad_any_cast` on mismatch.

**How `std::any` works.** This is the most important idea in the project:

```cpp
class MiniAny {
    void *_ptr = nullptr;
    const std::type_info *_type = nullptr;
    void (*_destroy)(void *) = nullptr;
    void *(*_copy)(const void *) = nullptr;

public:
    template <class T>
    MiniAny(T value)                                   // T is known HERE, and only here
        : _ptr(new T(std::move(value))),
          _type(&typeid(T)),
          _destroy([](void *p) { delete static_cast<T *>(p); }),
          _copy([](const void *p) -> void * { return new T(*static_cast<const T *>(p)); })
    {}

    ~MiniAny() { if (_destroy) _destroy(_ptr); }      // T unknown here, _destroy knows it

    template <class T>
    T *cast() { return *_type == typeid(T) ? static_cast<T *>(_ptr) : nullptr; }
};
```

A **template constructor** runs at the only moment `T` is known, and generates
small functions specialized for `T` ("destroy a T", "copy a T"), stored as plain
function pointers. Code that doesn't know `T` later calls those pointers.
**Capture the type-specific behavior while the type is known, and keep it
behind a type-agnostic interface.** `std::function` is the same trick applied to
callables. Steps 4 and 6 use it again.

### The code

```cpp
class Registry {
public:
    template <class Component>
    SparseArray<Component> &registerComponent();

    template <class Component>
    SparseArray<Component> &getComponents();            // throws std::out_of_range

    template <class Component>
    const SparseArray<Component> &getComponents() const;

private:
    std::unordered_map<std::type_index, std::any> _componentsArrays;
};

template <class Component>
SparseArray<Component> &Registry::registerComponent()
{
    auto it = _componentsArrays.try_emplace(
        std::type_index(typeid(Component)),
        std::in_place_type<SparseArray<Component>>).first;

    return std::any_cast<SparseArray<Component> &>(it->second);
}

template <class Component>
SparseArray<Component> &Registry::getComponents()
{
    auto it = _componentsArrays.find(std::type_index(typeid(Component)));

    if (it == _componentsArrays.end())
        throw std::out_of_range(
            std::string("ecs::Registry: component not registered: ") + typeid(Component).name());
    return std::any_cast<SparseArray<Component> &>(it->second);
}
```

- **`try_emplace`** constructs the value only if the key is absent, which makes
  `registerComponent` **idempotent**: registering twice returns the existing
  array with its data. (The alternatives were throwing, or silently wiping
  every component of that type.)
- **`std::in_place_type<SparseArray<Component>>`** builds the array directly
  inside the `any`, instead of building a temporary and moving it in.
- **The `any_cast` trap:** `any_cast<SparseArray<C> &>` returns a reference;
  `any_cast<SparseArray<C>>` returns a **copy**. Systems would modify the copy,
  and nothing would warn you.
- **`find`, not `operator[]`:** `operator[]` would *insert* an empty `any` for a
  missing key, then the cast would throw a confusing `bad_any_cast`, and the
  `const` overload would be impossible.
- **The `const` overload** (`any_cast<const SparseArray<C> &>`) gives read-only
  access through a `const Registry &`.
- **The registry does the cast, not the systems:** keeping `std::any` behind the
  registry's interface means the storage strategy can change later without
  touching any system. (It did, in steps 9 and 10.)

### Things the tests document

- **References stay valid while more types are registered:** `unordered_map` is
  node-based, so elements never move when it rehashes. (With a
  `vector<std::any>`, adding a type could reallocate and leave every
  `auto &positions = ...` dangling.) The test registers 64 `Tag<N>` types with
  a template lambda and a fold expression:
  ```cpp
  [&r]<std::size_t... I>(std::index_sequence<I...>) {
      (r.registerComponent<Tag<I>>(), ...);
  }(std::make_index_sequence<64>{});
  ```
- **Type aliases share a column:** `using Health = int; using Score = int;` is
  one type, so one key and one column. Components must be distinct structs.

### Limits of `std::any`

- **Every stored type must be copyable**, because `std::any` is copyable and
  must generate the "copy" function. With our design, every component must be
  copyable: `struct Sprite { std::unique_ptr<int> texture; };` fails to compile
  deep inside `std::any`'s manager.
- A **redundant type check** on every `any_cast` (the key already guarantees
  the type), and an extra **heap indirection** (a 24-byte array doesn't fit in
  libstdc++'s one-pointer inline buffer).

All three are fixed by step 10.

### Benchmark: the price of a lookup by type

**Claim to verify:** `getComponents<T>()` is cheap enough to call from every
system every frame. This is also the **baseline for step 10**, which replaces
the hash of the type name with an integer index.

`RegistryBenchmarks.cpp`:

```cpp
void BM_Registry_GetComponents(benchmark::State &state)
{
    ecs::Registry r;
    // A realistic number of component types: 20 others + Position.
    [&r]<std::size_t... I>(std::index_sequence<I...>) {
        (r.registerComponent<Tag<I>>(), ...);
    }(std::make_index_sequence<20>{});
    r.registerComponent<Position>();

    for (auto _ : state)
        benchmark::DoNotOptimize(&r.getComponents<Position>());
}
BENCHMARK(BM_Registry_GetComponents);
```

**Result:** **33.8 ns** per lookup. A frame with 10 systems × 3 lookups costs
about 1 µs, which is 0.006% of a 16.6 ms frame: acceptable. But 34 ns is the time to
hash a mangled name and walk a bucket. Step 10 should bring it down to a few
nanoseconds.

### Check yourself

`register` returns an empty array; `get` returns the same object (compare
addresses); writes are visible through `get`; each type has its own array;
registering twice keeps data; `get` on an unregistered type throws
`std::out_of_range`; the `const` overload works; references survive 64
registrations; aliases share a column.

---

## Step 4: Entity lifecycle

**Bootstrap step 2.3.** Files: `Registry.cpp` (new), `Entity.hpp`, `Registry.hpp/.tpp`, `tests/ECS/RegistryEntityTests.cpp`.

### Creating entities: a counter plus a free list

Ids are array indices. If ids only ever grow, the 5000th bullet makes every
array 5001 slots long. **Recycle dead ids** so arrays stay as large as the
maximum number of entities alive *at the same time*:

```cpp
Entity Registry::spawnEntity()
{
    if (!_freeIds.empty()) {
        Entity::id_type id = _freeIds.back();

        _freeIds.pop_back();
        _alive[id] = true;
        return Entity{id};
    }
    _alive.push_back(true);
    return Entity{_alive.size() - 1};
}
```

- `_freeIds` is a stack (LIFO): O(1), and the most recently freed id is the most
  likely to still be in cache.
- `std::vector<bool> _alive` has two jobs. Its size is the next never-used id,
  and it **guards double kills**: without it, killing twice pushes the id twice
  and two later spawns share a row. (`vector<bool>` packs bits and returns a
  proxy from `operator[]`: never take a `bool *` into it.)

### The stale handle problem (not solved yet)

```cpp
Entity enemy = r.spawnEntity();   // id 5
Entity target = enemy;            // a homing missile remembers it
r.killEntity(enemy);              // id 5 is freed
Entity bullet = r.spawnEntity();  // id 5 again
// `target` now designates the bullet, and isAlive(target) is true
```

This is the ABA problem. The fix (step 11) is **generational ids**. Until then,
don't store entities long-term.

### Only the registry creates entities: `friend`

```cpp
class Registry;

class Entity {
public:
    using id_type = std::size_t;
    constexpr operator id_type() const noexcept { return _id; }

private:
    constexpr explicit Entity(id_type id) noexcept : _id(id) {}
    friend class Registry;
    id_type _id;
};
```

Copying stays public (handles can be passed around and stored); creating one is
reserved to `Registry`. `explicit` still matters inside `Registry`. The forward
declaration `class Registry;` avoids a circular include.
`Registry::entityFromIndex(std::size_t)` is the **official, searchable door**
for turning a raw index (a loop counter, a network id) back into an entity. It
deliberately doesn't check `isAlive`.

### `killEntity`: erasing components whose types are unknown

`killEntity` must call `erase(e)` on every array, but they're behind `std::any`
and `killEntity` isn't a template. Apply the `MiniAny` trick: the only place
that knows `Component` is `registerComponent<Component>()`, so generate the
eraser there:

```cpp
template <class Component>
SparseArray<Component> &Registry::registerComponent()
{
    _erasers.reserve(_erasers.size() + 1);     // push_back below can't throw now

    auto [it, inserted] = _componentsArrays.try_emplace(
        std::type_index(typeid(Component)), std::in_place_type<SparseArray<Component>>);

    if (inserted)                              // once per type, or kills erase twice
        _erasers.push_back([](Registry &r, const Entity &e) {
            r.getComponents<Component>().erase(e);
        });
    return std::any_cast<SparseArray<Component> &>(it->second);
}

void Registry::killEntity(const Entity &e)
{
    if (!isAlive(e))
        return;
    for (const auto &eraser : _erasers)
        eraser(*this, e);
    _alive[e] = false;
    _freeIds.push_back(e);
}
```

with `using eraser_t = std::function<void(Registry &, const Entity &)>;
std::vector<eraser_t> _erasers;`.

- **The lambda takes `Registry &` instead of capturing `this`.** A captured
  `this` points to the registry that existed at registration. Copy the registry
  and the copy's erasers would erase from the **original**. The test
  `CopiedRegistryIsIndependent` kills in a copy and checks the original.
  **Rule: a callback stored inside an object must not capture `this` if the
  object can be copied or moved.**
- **`reserve` first:** if `try_emplace` succeeds and `push_back` then throws, an
  array would have no eraser, and its components would survive kills ("ghost
  components"). Reserving makes both succeed or neither (strong exception
  guarantee).
- The lambda captures nothing, so a plain function pointer would do. We use
  `std::function` as the bootstrap does.

### `addComponent`, `emplaceComponent`, `removeComponent`

```cpp
template <typename Component>
std::remove_cvref_t<Component> &addComponent(const Entity &to, Component &&c);   // final form (step 9)

template <typename Component, typename... Params>
Component &emplaceComponent(const Entity &to, Params &&...p);

template <typename Component>
void removeComponent(const Entity &from);
```

- `Component &&` with `Component` deduced is a **forwarding reference**. For an
  lvalue `Position p`, `Component` is deduced as `Position &`, and
  `getComponents<Position &>()` would instantiate `SparseArray<Position &>`,
  i.e. `std::optional<Position &>`, which is illegal (*"non-static data member
  in a union may not have reference type"*).
  **`std::remove_cvref_t<Component>`** strips `&` and `const` to name the
  column, while `std::forward<Component>(c)` still uses the original type to
  pick copy or move.
- `emplaceComponent<Position>(e, 1.f, 2.f)`: `Component` can't be deduced (no
  argument has that type), so it's given explicitly. Explicit arguments fill
  template parameters from the left, so `Component` comes first and the
  deduced pack `Params...` last.
- `assert(isAlive(to))` in add/emplace: adding to a dead id creates a
  component that survives until the id is reused, then appears on the new
  entity. That's a programmer error, so an `assert` rather than an exception.

### Modifying the world while iterating

With `SparseArray`, killing during iteration is safe (`erase` only resets an
optional). **Spawning or adding during iteration is not:** `insertAt` can grow
the array and reallocate it, and references such as `auto &pos = positions[i]`
dangle. Rule: don't keep component references across a spawn or add.

`Registry.cpp` now exists, so `ecs` becomes a `STATIC` library.

### Benchmark: what a kill costs, and why

**Claim to verify:** `killEntity` calls **every** eraser, one per registered
type, even for components the entity doesn't have. So its cost should grow
linearly with the number of registered types.

`RegistryBenchmarks.cpp` (the number of extra types is a template argument,
because `Tag<I>` types must be known at compile time):

```cpp
template <std::size_t Types>
void BM_Registry_SpawnKill(benchmark::State &state)
{
    ecs::Registry r;
    [&r]<std::size_t... I>(std::index_sequence<I...>) {
        (r.registerComponent<Tag<I>>(), ...);
    }(std::make_index_sequence<Types>{});
    r.registerComponent<Position>();

    for (auto _ : state) {                 // the life of a bullet, minus the game
        auto e = r.spawnEntity();
        r.addComponent(e, Position{1.f, 2.f});
        r.killEntity(e);
    }
}
BENCHMARK_TEMPLATE(BM_Registry_SpawnKill, 1);
BENCHMARK_TEMPLATE(BM_Registry_SpawnKill, 8);
BENCHMARK_TEMPLATE(BM_Registry_SpawnKill, 32);
```

**Result:**

| extra registered types | 1 | 8 | 32 |
|---|---|---|---|
| spawn + add + kill | 142 ns | 502 ns | 1697 ns |

That's linear: about **50 ns per registered type**, which is one `std::function` call
plus one hash lookup in each eraser. With 30 types and a few hundred bullets per
second, that's still microseconds per second: fine for now. It's the baseline for
step 10, where the kill calls a virtual `erase` directly on each storage, with no
lookup.

### Check yourself

Consecutive ids; LIFO reuse; `isAlive`; double kill doesn't duplicate an id;
killing a never-spawned id is a no-op; kill removes all components of that
entity only; a respawned id has no ghost components; kill works for types
registered after the spawn; add from lvalue copies (count copies), from const
lvalue works, from rvalue never copies; emplace builds in place; remove keeps
the others; adding an unregistered component throws; a copied registry is
independent.

---

## Step 5: Using the ECS, the SFML demo

**Bootstrap step 3.** Files: `src/Bootstrap/*`.

### Components: data only

```cpp
namespace component {

struct Position { float x; float y; };         // top-left, pixels
struct Velocity { float vx; float vy; };       // pixels per frame
struct Drawable {                              // what to draw (not where)
    sf::Vector2f size;
    sf::Color color;
    int layer = 0;                             // added in step 9
};
struct Controllable { float speed; };          // keyboard drives Velocity
struct Wrap {};                                // tag: wrap around screen edges

}
```

- Components belong to the **game** (`src/`), not to the ECS library. The
  dependency goes one way: game → ECS.
- **`Drawable` does not hold an `sf::RectangleShape`:** the shape stores its
  own position, so there would be two positions to keep in sync (one source of
  truth is the rule). It's also heavy, and ties the component to a library.
- `Controllable` carries the speed, so two controllable entities can differ.
- **Tag components** (empty structs) mean "this entity opts into a behavior".

### Systems: one pattern

First version (before step 6), every system fetches its columns:

```cpp
void positionSystem(ecs::Registry &r)
{
    auto &positions = r.getComponents<component::Position>();
    const auto &velocities = std::as_const(r).getComponents<component::Velocity>();

    const std::size_t count = std::min(positions.size(), velocities.size());
    for (std::size_t i = 0; i < count; ++i) {
        auto &pos = positions[i];
        const auto &vel = velocities[i];
        if (pos && vel) {
            pos->x += vel->vx;
            pos->y += vel->vy;
        }
    }
}
```

- **`std::min` of the sizes:** each array is as long as the highest id that has
  that component, so they differ. An entity needs both, so stop at the shorter.
- **`std::as_const(r)`** selects the `const` overload of `getComponents`.
  Writing to `velocities` becomes a compile error, and the first lines of the
  system document its **read set** and **write set**. Schedulers use those to
  run systems in parallel.
- The keyboard is read **once per frame**, outside the loop:
  `dx = isKeyPressed(Right) - isKeyPressed(Left)` gives -1/0/+1 without branches.

### Three forms of system

| Form | When | Here |
|---|---|---|
| Free function | needs nothing else | control, position, logging |
| Function object (class with `operator()`) | needs state or configuration | `DrawSystem` (render target, reused shape), `WrapSystem` (screen size) |
| Lambda | small, or captures a local | tests |

A lambda *is* a function object generated by the compiler. `DrawSystem` stores an
`sf::RenderTarget &` (the base class of windows and render textures) rather
than an `sf::RenderWindow &`, and reuses one `sf::RectangleShape`.

### The game loop

```cpp
while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {    // SFML 3 events
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) { ... }
    }
    controlSystem(registry);   // keys → Velocity
    positionSystem(registry);  // Velocity → Position
    wrapSystem(registry);      // Position → Position
    window.clear();
    drawSystem(registry);      // Position + Drawable → pixels
    window.display();
}
```

Systems communicate **only through components** and never call each other.
The cost is that **call order is part of the game logic**: position before control
means one frame of input lag. SFML 3: `pollEvent()` returns
`std::optional<sf::Event>`, `is<T>()` tests the type, `getIf<T>()` returns a
pointer or `nullptr`. `loggingSystem` runs when L is pressed, since a system is just
a function.

### Entities as compositions

| Entity | Position | Velocity | Drawable | Controllable | Wrap |
|---|:-:|:-:|:-:|:-:|:-:|
| 4 walls (gray) | ✓ | | ✓ | | |
| 3 drifters (red) | ✓ | ✓ | ✓ | | ✓ |
| player (green, layer 1) | ✓ | ✓ | ✓ | ✓ | ✓ |

`Wrap` is written once and works for drifters and player. **Adding a feature =
adding a component + a system, without editing existing code.** The bootstrap's
acceleration question works the same way: an `Acceleration` component, have
`controlSystem` write it, and an `accelerationSystem` (`vel += acc`) running
between control and position. Nothing in the ECS changes.

### Known flaws at this point

- **Draw order was "id order"** (players had to be spawned last). It broke in
  step 9 and was fixed with explicit layers.
- **Speed depends on frame rate** (pixels per frame, hidden by
  `setFramerateLimit(60)`). Fix: delta time, provided as a *resource* (step 12).
- Boilerplate in every system and hidden dependencies, which motivate steps 6 and 7.

### CMake

```cmake
find_package(SFML 3.1 REQUIRED COMPONENTS Graphics)
add_executable(bootstrap main.cpp Systems.cpp)
target_include_directories(bootstrap PRIVATE include)
target_link_libraries(bootstrap PRIVATE warning_options ecs SFML::Graphics)
set_target_properties(bootstrap PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
```

`3.1` is a minimum version within major version 3. `REQUIRED` fails at
configure time with a clear message. `BUILD_BOOTSTRAP=OFF` builds the ECS and
tests without SFML. (Alternative for machines without SFML installed:
`FetchContent_Declare(SFML GIT_REPOSITORY ... GIT_TAG 3.1.0 GIT_SHALLOW ON
EXCLUDE_FROM_ALL SYSTEM)`. It still needs the X11/OpenGL/udev headers.)

---

## Step 6: Systems inside the registry

**Bootstrap step 4.1.** Files: `Registry.hpp/.tpp/.cpp`, `tests/ECS/RegistrySystemTests.cpp`.

### Goal

Systems **declare** their data in their signature, and the registry provides it:

```cpp
void positionSystem(Registry &r,
    SparseArray<Position> &positions,              // writes   (SparseSet since step 9)
    const SparseArray<Velocity> &velocities);      // reads

r.addSystem<Position, Velocity>(positionSystem);   // once
r.runSystems();                                    // every frame, in insertion order
```

### The implementation

```cpp
template <class... Components, typename Function>
void Registry::addSystem(Function &&f)
{
    static_assert((std::is_same_v<Components, std::remove_cvref_t<Components>> && ...),
        "addSystem<Components...>: use plain component types. Read-only access is "
        "declared in the system itself, with a const SparseArray<T> & parameter");
    static_assert(std::is_invocable_v<std::decay_t<Function> &, Registry &, SparseArray<Components> &...>,
        "addSystem<Components...>(f): f must be callable as "
        "f(Registry &, SparseArray<Components> &...), in that order");
    assert(!_runningSystems && "addSystem called from inside a running system");

    (registerComponent<Components>(), ...);

    _systems.emplace_back([f = std::forward<Function>(f)](Registry &r) mutable {
        f(r, r.getComponents<Components>()...);
    });
}

void Registry::runSystems()
{
    struct RunningGuard {
        bool &flag;
        ~RunningGuard() { flag = false; }
    } guard{_runningSystems};

    _runningSystems = true;
    for (auto &system : _systems)
        system(*this);
}
```

with `using system_t = std::function<void(Registry &)>; std::vector<system_t> _systems; bool _runningSystems = false;`.

### Every feature used here

**A pack before another parameter.** `template <class... Components, typename
Function>` is allowed for function templates when what follows the pack can be
deduced. In `addSystem<Position, Velocity>(f)`, the explicit arguments fill
`Components...` and `Function` is deduced from `f`. With `Function` first, you'd
have to name a lambda's type, which is impossible.

**Type erasure, third time.** The adapter lambda converts "a system wanting
specific arrays" into "something called with the registry". It is generated
where `Components...` and `Function` are known; `std::function<void(Registry
&)>` erases its unique type; `runSystems` knows no types.

| Step | What is erased | Captured while the type is known | Called by |
|---|---|---|---|
| 3 `std::any` | the array's type | copy / destroy / type check | the map, `any_cast` |
| 4 erasers | the component type | "erase my T for this entity" | `killEntity` |
| 6 systems | system type + its component list | "fetch my arrays and call me" | `runSystems` |

**Pack expansion.** `...` after an expression containing a pack repeats the
expression (the *pattern*) for each element, in order, at compile time:
`f(r, r.getComponents<Components>()...)` becomes `f(r,
r.getComponents<Position>(), r.getComponents<Velocity>())`. The position of `...`
matters: `getComponents<Components...>()` would be a single call with every type.

**Init-capture + perfect forwarding.** `[f = std::forward<Function>(f)]` creates
a member `f`, copied from an lvalue and moved from an rvalue. Init-captures
deduce like `auto`, so references and `const` are dropped and functions decay
to function pointers. The stored system is always **owned by the registry**.
One forwarding-reference overload covers all cases; the bootstrap's second
`const Function &` overload adds nothing.

**`mutable`** makes the lambda's `operator()` non-const, so it can call a
non-const `operator()` (e.g. `DrawSystem`, which modifies its shape). Stateful
systems keep their state between frames.

**No caching of arrays.** Capturing references to the arrays in `addSystem`
would tie the system to that registry. Copies of the registry would update the
original's arrays (the same trap as the erasers). Fetching through the `Registry &`
parameter each run costs about 10 hash lookups per frame, which is negligible.

**Fold expressions (C++17).** `( pattern OP ... )` with any binary operator:
- `(registerComponent<Components>(), ...)`: comma operator, a left-to-right
  "for each type" (registers missing arrays; idempotent).
- `(std::is_same_v<...> && ...)`: "true for every type".
- The parentheses around the whole fold are mandatory, and each operand must be
  a primary expression: write `((a != b) && ...)`, not `(a != b && ...)`
  (compile error, found in step 9).

**`static_assert` + `std::is_invocable`** check at the entry point and give one
clear message instead of pages of template errors. Note the pack expansion in a
type list: `SparseArray<Components> &...`. We check `std::decay_t<Function> &`
because that's what the lambda calls: its own decayed copy, as a non-const
lvalue.

**Why not `addSystem<Position, const Velocity>`:** `typeid` ignores top-level
`const`, so `getComponents<const Velocity>()` would find Velocity's array and
then fail the `any_cast` at runtime. Read-only access is declared in the
**system's parameter** (`const SparseArray<Velocity> &`). Binding a non-const
array to a const reference parameter is implicit.

**The RAII guard.** If a system calls `addSystem` while `runSystems` iterates,
`emplace_back` may reallocate `_systems` and move the `std::function` that is
currently running. The flag + `assert` detects that. The guard's destructor
resets the flag even when a system throws (RAII, like `std::lock_guard`).

**Game side.** Systems lose their fetching code; unused `Registry &` parameters
are left unnamed (no `-Wunused-parameter`). `DrawSystem` is added last, so the
loop becomes `clear(); runSystems(); display();`. `loggingSystem` is still a
plain function, called directly with `registry.getComponents<...>()` arguments.

### Limits after this step

The boilerplate loop is still in every system (step 7). `std::function` requires
**copyable** callables, so a lambda capturing a `unique_ptr` is rejected (C++23
`std::move_only_function`, or step 10). Every row is still visited.

### Benchmark: the overhead of `runSystems`

**Claim to verify:** storing systems behind `std::function` and fetching their
arrays at every run costs nothing that matters compared to a frame.

`RegistryBenchmarks.cpp`:

```cpp
void BM_Registry_RunSystems(benchmark::State &state)
{
    ecs::Registry r;
    for (int64_t i = 0; i < state.range(0); ++i)
        r.addSystem<Position, Small>([](ecs::Registry &, auto &, const auto &) {});   // empty body

    for (auto _ : state)
        r.runSystems();
    state.SetItemsProcessed(state.iterations() * state.range(0));   // items = system calls
}
BENCHMARK(BM_Registry_RunSystems)->Arg(1)->Arg(10)->Arg(50);
```

The systems do nothing, so we measure **only the dispatch**: the
`std::function` call, two `getComponents` lookups, and the call to the system.
(A generic lambda works with `addSystem`: `std::is_invocable` instantiates it
with the real argument types.)

**Result:** about **70 ns per system**, constant (72 ns for 1, 703 ns for 10,
3.6 µs for 50; about 14 million system calls per second). 50 systems cost 0.02% of a
60 FPS frame. **Conclusion: the dispatch design is fine. Performance is decided
by the loops inside the systems**, which is what steps 7–9 work on.

### Check yourself

Free function, function pointer, capturing lambda, `addSystem<>` with no
components, insertion order, an lvalue functor is copied (the original's counter
stays 0), a stateful functor keeps state across runs, missing components get
registered, systems can spawn/kill, a copied registry runs systems on itself,
a throwing system doesn't leave `addSystem` locked.

---

## Step 7: The zipper

**Bootstrap step 4.2.** Files: `ZipperIterator.hpp/.tpp`, `Zipper.hpp/.tpp`,
`IndexedZipper.hpp/.tpp`, `tests/ECS/ZipperTests.cpp`.

### Goal

```cpp
for (auto &&[pos, vel] : Zipper(positions, velocities)) {
    pos.x += vel.vx;
    pos.y += vel.vy;
}
```

### What a range-for needs

```cpp
for (auto &&[pos, vel] : Zipper(positions, velocities)) { body }
// is rewritten by the compiler as:
{
    auto &&range = Zipper(positions, velocities);
    auto it = range.begin();
    auto end = range.end();
    for (; it != end; ++it) {
        auto &&[pos, vel] = *it;
        body
    }
}
```

So: a type with `begin()`/`end()`, and an iterator with `!=`, prefix `++`, `*`.

### The iterator's type aliases

An iterator is any type with the right operations and aliases (a *named
requirement*; since C++20, also checkable *concepts*):

| Alias | Ours | Why |
|---|---|---|
| `value_type` | `std::tuple<Position &, const Velocity &>` | computed from the containers |
| `reference` | same tuple, returned **by value** | it's a **proxy**: no such tuple exists in memory, `*it` builds one |
| `pointer` | `void` | no `operator->`: there's nothing to point to |
| `difference_type` | `std::ptrdiff_t` | must be **signed** (the bootstrap's `size_t` is wrong) |
| `iterator_category` | `std::input_iterator_tag` | legacy *forward* iterators require `reference` to be a real `T &` |
| `iterator_concept` | `std::forward_iterator_tag` | C++20 concepts accept proxies: for `std::ranges` it's forward |

**Departures from the bootstrap's skeleton:** no `operator->` (the skeleton's
`value_type operator->()` can't work); `operator++(int)` returns a **copy** (the
old position is a different object); `operator*` is `const` (required by
`std::indirectly_readable`); a public default constructor (required by every
C++20 iterator concept); only `operator==` (C++20 rewrites `a != b` as
`!(a == b)`).

### Computing types with `decltype` and `std::declval`

```cpp
template <class Container>
using iterator_t = decltype(std::declval<Container &>().begin());
template <class Container>
using it_reference_t = std::iterator_traits<iterator_t<Container>>::reference;

using value_type = std::tuple<decltype(*std::declval<it_reference_t<Containers>>())...>;
```

`std::declval<T>()` pretends to produce a `T` inside unevaluated contexts;
`decltype` gives an expression's type without evaluating it.
For a `const SparseArray<Velocity>`: `begin()` returns a `const_iterator`, `*it` is a
`const std::optional<Velocity> &`, and `*` on that is a `const Velocity &`.
**Constness carries through automatically.**

### Looping over a tuple: `std::index_sequence`

`std::get<N>` needs `N` at compile time, so a runtime `for` can't visit a tuple.
`std::index_sequence<0, 1, 2>` carries the indices in its type; a function
taking it deduces the pack `Is...` and folds over it:

```cpp
static constexpr std::index_sequence_for<Containers...> _seq{};   // <0, 1, ..., N-1>

template <std::size_t... Is>
bool allSet(std::index_sequence<Is...>) const
{
    return (static_cast<bool>(std::get<Is>(_begins)[_idx]) && ...);
}

template <std::size_t... Is>
reference toValue(std::index_sequence<Is...>) const
{
    return reference{*std::get<Is>(_begins)[_idx]...};   // [_idx] → optional, * → component
}
```

(`std::apply` is the C++17 shortcut built on the same mechanism.)

### The algorithm

```cpp
// constructor (begin): if the first row is incomplete, advance immediately
if (_idx < _max && !allSet(_seq))
    ++*this;

ZipperIterator &operator++()
{
    assert(_idx < _max);
    do {
        ++_idx;
    } while (_idx < _max && !allSet(_seq));
    return *this;
}
// equality: lhs._idx == rhs._idx; end() is an iterator with _idx == _max
```

`_max` is the size of the **shortest** array: past it, some arrays have no
slot.

### The Zipper class

```cpp
template <class... Containers>
class Zipper {
public:
    using iterator = ZipperIterator<Containers...>;
    explicit Zipper(Containers &...cs) : _begin(cs.begin()...), _size(computeSize(cs...)) {}
    iterator begin() const { return iterator{_begin, 0, _size}; }
    iterator end() const { return iterator{_begin, _size, _size}; }
private:
    static std::size_t computeSize(Containers &...cs)
    {
        return std::min({static_cast<std::size_t>(cs.size())...});   // initializer_list from a pack
    }
    iterator_tuple _begin;
    std::size_t _size;
};
```

- **CTAD** (class template argument deduction): `Zipper(positions, velocities)`
  deduces `Containers...`, including `const`.
- **`Containers &...` only binds lvalues:** zipping a temporary array would
  leave dangling iterators, so it's rejected at compile time.
- `begin`/`end` are `const`, which makes `const Zipper` a range too.

### Performance: measure, don't assume

1M entities, half with a Velocity, `-O2`, best of 5 runs:

| Iterator design | GCC 13 | Clang 18 |
|---|---|---|
| Bootstrap design: one iterator per array + an index, all advanced by `++` | **1.96×** the hand-written loop | 0.99× |
| **One index**, read `std::get<Is>(_begins)[_idx]` | **1.07×** | 0.98× |

GCC didn't reduce N+1 parallel counters to one; with a single index the code
has the shape of the hand-written loop. Consequences: the bootstrap's
`_compute_end` disappears (the end is just `index == size`), and the iterators
must be random access. **"Zero cost" is a goal to verify, per compiler.**

### Benchmark: the zipper against the loop it replaces

**Claim to verify:** the zipper costs about the same as the hand-written loop
from step 5.

`IterationBenchmarks.cpp`. Both benchmarks build the same data (`Position` on
every entity, `T` on a density of them) and compute the same sum:

```cpp
template <class T>
void BM_Join_SparseArrayManual(benchmark::State &state)
{
    ecs::SparseArray<Position> positions;
    ecs::SparseArray<T> others;
    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        positions.insertAt(e, Position{1.f, 0.f});
    for (std::size_t e : pickEntities(densityOf(state)))
        others.insertAt(e, T{1.f, {}});

    for (auto _ : state) {
        float sum = 0.f;
        const std::size_t count = std::min(positions.size(), others.size());
        for (std::size_t i = 0; i < count; ++i)
            if (positions[i] && others[i])
                sum += positions[i]->x * others[i]->value;
        benchmark::DoNotOptimize(sum);
    }
}

template <class T>
void BM_Join_Zipper(benchmark::State &state)
{
    // ... same setup ...
    for (auto _ : state) {
        float sum = 0.f;
        for (auto &&[pos, other] : ecs::Zipper(positions, others))
            sum += pos.x * other.value;
        benchmark::DoNotOptimize(sum);
    }
}
#define DENSITIES ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000)->Unit(benchmark::kMillisecond)
BENCHMARK_TEMPLATE(BM_Join_SparseArrayManual, Small) DENSITIES;
BENCHMARK_TEMPLATE(BM_Join_Zipper, Small) DENSITIES;
```

Then: `compare.py filters $B 'BM_Join_SparseArrayManual' 'BM_Join_Zipper'`.

**To reproduce the design comparison** (the 1.96× above), keep a copy of the
bootstrap-style iterator (`++` advances one iterator per array) in the
benchmarks directory as `PdfZipper` and add a third benchmark. Comparing
designs inside the same harness is what makes the conclusion solid.

**Results:** the hand-rolled harness gave 1.07× (GCC) and 0.98× (Clang) at 50%.
Google Benchmark on GCC `-O2`, median of 5 repetitions (cv < 3%), gave
**1.12× at 1%, 1.33× at 50%, 1.26× at 100%**. The zipper is close to the
loop, but not free, and the exact ratio depends on the harness and density.
What stays solid is the **comparison between designs**, measured in the same
harness: the bootstrap's design was about 2× slower than the one-index design
with GCC.

### Structured bindings and `IndexedZipper`

- `auto &&[pos, vel]` binds the temporary tuple; `pos` names a `Position &`.
  **`auto [pos, vel]` also modifies the arrays:** copying a tuple of references
  copies references, not objects.
- `[[maybe_unused]] auto &&[pos, drawable, wrap]` when a component is only
  used as a filter (tags).
- `IndexedZipper` **wraps** a `ZipperIterator` (composition, not copy-paste):
  `*it` returns `std::tuple_cat(std::tuple<std::size_t>{inner.index()},
  *inner)`, i.e. `std::tuple<std::size_t, Position &, const Velocity &>`.

Because the iterator satisfies the standard concepts, `std::for_each`,
`std::ranges::count_if` and `std::ranges::distance` all work on it.

### Safety rules

The arrays must outlive the Zipper; don't add components while iterating
(reallocation invalidates the stored `begin()`s); killing is fine with
`SparseArray`.

### Check yourself

Compile-time: `reference` types, `std::forward_iterator`,
`std::ranges::forward_range<const Zipper<...>>`, CTAD result. Runtime: only
complete rows, writes go through, `auto` bindings still reference, empty arrays,
only holes, begin skips leading holes, stops at the shortest array, single
array, three arrays, post-increment, standard algorithms, const arrays give
const references, indexed zipper gives the right ids.

---

## Step 8: Sparse sets and the memory footprint

**Bootstrap step 4.3.** Files: `SparseSet.hpp/.tpp`, `tests/ECS/SparseSetTests.cpp`, `benchmarks/*`.

### The problem, measured

1M entities, a 64-byte component on 0.1% of them: the `SparseArray` uses
**68 MB** (99.9% holes) and iterating those 1000 components takes **6–9 ms**,
because it checks 1,000,000 slots. Cost follows the highest id, not the number
of components.

### The sparse set

```
entity:        0     1     2     3     4
_sparse:    [npos,   1,  npos,   0,  npos]     entity → position in the packed arrays
_dense:       [3,    1]                        position → entity
_components:  [C3,   C1]                       position → component
```

```cpp
template <typename Component>
class SparseSet {
public:
    using size_type = std::size_t;
    using entity_type = std::size_t;
    static constexpr size_type npos = static_cast<size_type>(-1);

    bool contains(entity_type e) const noexcept;        // e < _sparse.size() && _sparse[e] != npos
    Component &get(entity_type e);                      // asserted
    Component *tryGet(entity_type e) noexcept;          // nullptr if absent (+ const versions)
    Component &insertAt(entity_type e, const Component &c);
    Component &insertAt(entity_type e, Component &&c);
    template <class... Params> Component &emplaceAt(entity_type e, Params &&...p);
    void erase(entity_type e);
    size_type size() const noexcept;                    // number of components
    bool empty() const noexcept;
    std::span<const entity_type> entities() const noexcept;
    std::span<Component> components() noexcept;         // + const version

private:
    template <class... Params> Component &append(entity_type e, Params &&...p);

    std::vector<size_type> _sparse;
    std::vector<entity_type> _dense;
    std::vector<Component> _components;
};
```

**Erase = swap-and-pop, O(1):**

```cpp
const size_type hole = _sparse[e];
const size_type last = _dense.size() - 1;
if (hole != last) {
    _components[hole] = std::move(_components[last]);
    _dense[hole] = _dense[last];
    _sparse[_dense[hole]] = hole;      // fix the moved entity's index
}
_components.pop_back();
_dense.pop_back();
_sparse[e] = npos;
```

### Design decisions

- **Order is not stable, and erase moves the last component.** References to
  components can be invalidated by erasing *another* entity. Erasing while
  iterating forward skips elements. **Iterate backwards:** the element swapped
  into the current slot comes from the end, which has already been visited
  (that's what EnTT does).
- **Replacing in `emplaceAt` builds a temporary and moves it in**
  (`_components[i] = Component(std::forward<Params>(p)...)`). Destroy-then-construct
  is fine for `optional` (it can become empty if the constructor throws), but a
  packed array has no empty state.
- **`append` keeps the three arrays consistent (strong guarantee):**
  1. grow `_sparse` with `npos` (harmless if we stop),
  2. `_components.emplace_back(...)` (if it throws, nothing else changed),
  3. `_dense.push_back(e)` inside `try`; on failure `_components.pop_back()` and rethrow,
  4. `_sparse[e] = _dense.size() - 1` (can't throw).
- `std::vector::emplace_back` handles arguments that point into the vector
  itself, so `set.insertAt(e, set.get(0))` is safe.
- **`std::span`** (C++20) is a non-owning pointer + size. Returning spans instead
  of `vector &` stops callers from resizing the arrays and breaking the
  invariants.
- **`get` vs `tryGet`** express intent ("must exist: bug otherwise" vs "may not
  exist"). Measured alternatives: `std::optional<std::reference_wrapper<T>>`
  costs the same as the pointer; `std::optional<T>` **copies**, which is up to
  ~6× slower for a `std::string` component and silently breaks writes.
  (`std::optional<T &>` arrives in C++26.)

### Benchmark results (GCC 13, `-O2`, 1M entities, best of 20)

Small component (8 bytes):

```
density |   memory (MB)   |  iterate (ms)   | join with Position (ms)
        |  array     set  |  array     set  |  array     set
   0.1% |  12.00    8.01  |  1.064   0.001  |  2.325   0.014
   1.0% |  12.00    8.16  |  1.181   0.010  |  2.260   0.129
  10.0% |  12.00    9.60  |  2.120   0.123  |  3.276   2.611
  50.0% |  12.00   16.00  |  6.452   0.681  |  7.445   4.063
 100.0% |  12.00   24.00  |  1.863   1.282  |  3.798   5.713
```

Big component (64 bytes): at 0.1%, 68 MB vs 8 MB, and iteration 9.3 ms vs 0.001 ms.

Lessons:
1. **Rare components: the set wins by 100–1000×** in time, and by a lot in memory for big components.
2. **The array's worst case is 50%, not 100%**: the `if (slot)` branch becomes a
   coin flip and the CPU mispredicts about half the time. The packed loop has no branch.
3. **At 100% density the array is better**: joins about 1.5× faster (no
   `_sparse` indirection), and half the memory for small components (the set pays
   8 + 8 bytes per entity on top of the component).
4. **Join strategy:** iterate the **smaller** set and probe the others.
5. The set still has an O(highest id) part: `_sparse` (8 MB for 1M ids). Fixes:
   32-bit indices, paged `_sparse`.

### Benchmarks for this step (Google Benchmark versions)

The table above comes from the hand-rolled `StorageBenchmark.cpp`. With Google
Benchmark, the same questions become three groups in `StorageBenchmarks.cpp` and
`IterationBenchmarks.cpp`.

**1. Density sweep: why the sparse set at all.** Write
`BM_SparseSet_Iterate<T>` (iterate `set.components()`) next to
`BM_SparseArray_Iterate<T>` from step 2, and the two joins (the array-based
manual join is `BM_Join_SparseArrayManual`, written in step 7):

```cpp
template <class T>
void BM_Join_SparseSetManual(benchmark::State &state)
{
    ecs::SparseSet<Position> positions;
    ecs::SparseSet<T> others;
    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        positions.insertAt(e, Position{1.f, 0.f});
    for (std::size_t e : pickEntities(densityOf(state)))
        others.insertAt(e, T{1.f, {}});

    for (auto _ : state) {
        float sum = 0.f;
        const auto entities = others.entities();       // walk the smaller set...
        const auto components = others.components();
        for (std::size_t i = 0; i < entities.size(); ++i)
            if (const Position *pos = positions.tryGet(entities[i]))   // ...probe the bigger one
                sum += pos->x * components[i].value;
        benchmark::DoNotOptimize(sum);
    }
    state.counters["matches"] = static_cast<double>(others.size());
}
```

Register both with `BENCHMARK_TEMPLATE(..., Small)` and `(..., Big)` over
densities 1, 10, 100, 500, 1000 ‰, add a memory counter computed as in the
table above, and compare with `compare.py filters $B BM_Join_SparseArrayManual
BM_Join_SparseSetManual`. Expected shape: the set wins by orders of magnitude at
low density, and the array wins about 1.5× at 100%.

**2. Swap-and-pop vs shifting: why erase is O(1).**

```cpp
void BM_Erase_VectorShift(benchmark::State &state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        state.PauseTiming();                        // rebuilding is not what we measure
        std::vector<Small> v(n, Small{1.f, 0.f});
        state.ResumeTiming();
        while (!v.empty())
            v.erase(v.begin());                     // shifts everything after it
        benchmark::DoNotOptimize(v);
    }
    state.SetComplexityN(state.range(0));
}

void BM_Erase_SwapAndPop(benchmark::State &state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        state.PauseTiming();
        ecs::SparseSet<Small> set;
        for (std::size_t e = 0; e < n; ++e)
            set.insertAt(e, Small{1.f, 0.f});
        state.ResumeTiming();
        for (std::size_t e = 0; e < n; ++e)
            set.erase(e);                           // always the first element: worst case for shifting
        benchmark::DoNotOptimize(set);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Erase_VectorShift)->RangeMultiplier(4)->Range(256, 16384)->Complexity();
BENCHMARK(BM_Erase_SwapAndPop)->RangeMultiplier(4)->Range(256, 16384)->Complexity();
```

**Result** (erasing all N elements):

| N | 256 | 1024 | 4096 | 16384 | fitted complexity |
|---|---|---|---|---|---|
| `vector::erase` (shift) | 7.4 µs | 86 µs | 1.4 ms | **40 ms** | super-linear (N erases × O(N) shift each) |
| swap-and-pop | 1.7 µs | 4.7 µs | 18.7 µs | **77 µs** | **O(N)**, i.e. O(1) per erase |

At 16k elements, swap-and-pop is about **500× faster**. That's the price of
keeping the order, and an ECS doesn't need the order.

**3. `tryGet` vs `std::optional<T>` (the question from this step):**

```cpp
void BM_Access_TryGet(benchmark::State &state)
{
    ecs::SparseSet<Named> set;                      // Named { std::string name; }, 32 chars
    for (std::size_t e = 0; e < 100'000; ++e)
        set.insertAt(e, Named{std::string(32, 'x')});

    for (auto _ : state) {
        std::size_t sum = 0;
        for (std::size_t e = 0; e < 100'000; ++e)
            if (const Named *n = set.tryGet(e))
                sum += n->name.size();
        benchmark::DoNotOptimize(sum);
    }
}
// BM_Access_OptionalCopy: same loop through a lambda returning std::optional<Named>.
```

**Result:** `tryGet` 327 µs, `optional<Named>` copy **2449 µs (7.5×)**: each
access copies the string, which means a heap allocation.

### The bootstrap's hybrid: `std::variant` + `std::visit`

`std::variant<SparseArray<T>, SparseSet<T>>` is a type-safe union (one
alternative alive at a time, a tag, no heap). `std::visit(f, v)` calls `f` with
the active alternative, through a table indexed by `v.index()`. The overloaded
idiom:

```cpp
template <class... Fs> struct Overloaded : Fs... { using Fs::operator()...; };

std::visit(Overloaded{
    [&](const SparseArray<T> &a) { /* array version */ },
    [&](const SparseSet<T> &s)   { /* set version */ },
}, storage);
```

**Rejected, with data:** the array only wins near 100% density (~1.5× on joins);
every access would pay a dispatch; joining N hybrid storages instantiates 2^N
combinations; switching layouts is an O(n) stutter in the middle of a frame (and
needs hysteresis); and the set's weak spots have cheaper fixes. **Decision: the
sparse set is the only storage** (EnTT's choice).

### Check yourself

Empty set; size counts components, not the highest id; packed insertion order;
replace; swap-and-pop order after erase; erase the last; erase missing is a
no-op; reinsert after erase; **erase while iterating backwards visits everything
once**; emplace of a new component builds in place (0 copies/moves);
emplace over an existing one costs one move; inserting a copy of its own
element while growing (ASan); **a throwing constructor leaves the set
unchanged** (new and replace); const access.

---

## Step 9: The registry on sparse sets, and views

Files: `View.hpp/.tpp`, `ViewIterator.hpp/.tpp`, `IndexedView.hpp/.tpp`,
`Registry.hpp/.tpp`, `tests/ECS/ViewTests.cpp`, the game systems.

### Registry changes

Only type names changed (`SparseArray<T>` → `SparseSet<T>` in
`registerComponent`, `getComponents`, the erasers and `addSystem`), thanks to
two earlier decisions: the registry does the casting (step 3), and the erasers
only call `erase(e)` (step 4). One API improvement:
`addComponent`/`emplaceComponent` now return the **component**
(`std::remove_cvref_t<Component> &` / `Component &`) instead of the optional
slot.

Behavior visible in the tests: `size()` counts components (inserting at id 2
gives size 1), `storage[e]` becomes `contains(e)` / `get(e)`, and killing while
looping over a set must go backwards:

```cpp
for (std::size_t i = positions.size(); i-- > 0;) {    // test i, then decrement
    const std::size_t e = positions.entities()[i];
    if (positions.get(e).x >= 2.f)
        r.killEntity(r.entityFromIndex(e));
}
```

### The View

```cpp
for (auto &&[pos, vel] : View(positions, velocities))        // same syntax as the Zipper
for (auto &&[e, pos, vel] : IndexedView(positions, velocities))
```

1. At construction, find the **smallest** set: the **driver**.
2. Walk the driver's packed `entities()` **backwards**, and keep the entities
   that every other set contains.

```cpp
template <class... Containers>
class View {
public:
    using iterator = ViewIterator<Containers...>;
    explicit View(Containers &...cs) : _sets(std::addressof(cs)...), _driver(smallest(cs...)) {}
    iterator begin() const { return iterator{_sets, _driver, static_cast<std::size_t>(-1)}; }  // clamped
    iterator end() const { return iterator{_sets, _driver, 0}; }
private:
    static std::size_t smallest(Containers &...cs)
    {
        const std::array<std::size_t, sizeof...(Containers)> sizes{cs.size()...};
        return static_cast<std::size_t>(std::ranges::min_element(sizes) - sizes.begin());
    }
    std::tuple<Containers *...> _sets;
    std::size_t _driver;
};
```

The iterator stores `_sets`, `_driver`, `_remaining` (driver elements left; the
current one is at `_remaining - 1`; 0 is the end) and `_current` (a tuple of
cached component pointers). Equality compares `_remaining`.

### Selecting a tuple element at runtime

`_driver` is a runtime value, but `std::get<I>` needs a constant. Fold over
every index and act only on the matching one:

```cpp
template <std::size_t... Is>
std::size_t driverSize(std::index_sequence<Is...>) const
{
    std::size_t size = 0;
    static_cast<void>(((Is == _driver && (size = std::get<Is>(_sets)->size(), true)) || ...));
    return size;
}
```

`&&` runs the assignment only where `Is == _driver`; `(assign, true)` uses the
comma operator to yield `true`, which stops the `||` chain; `static_cast<void>`
discards the result. `driverEntity(i)` uses the same pattern with
`entities()[i]`.

### Iteration

```cpp
ViewIterator(const sets_tuple &sets, std::size_t driver, std::size_t remaining)
    : _sets(sets), _driver(driver), _remaining(std::min(remaining, driverSize(_seq)))
{
    skipIncomplete();
}

ViewIterator &operator++()
{
    assert(_remaining > 0);
    _remaining = std::min(_remaining - 1, driverSize(_seq));   // the body may have shrunk the driver
    skipIncomplete();
    return *this;
}

void skipIncomplete()
{
    while (_remaining > 0) {
        const std::size_t i = _remaining - 1;
        if (fetchAll(driverEntity(i, _seq), i, _seq))
            return;
        _remaining = i;
    }
}

template <std::size_t... Is>
bool fetchAll(std::size_t e, std::size_t i, std::index_sequence<Is...>)
{
    // One lookup per set; the driver's component is read from its packed array.
    return (((std::get<Is>(_current) = Is == _driver
                  ? &std::get<Is>(_sets)->components()[i]
                  : std::get<Is>(_sets)->tryGet(e)) != nullptr) && ...);
}

reference operator*() const { return reference{*std::get<Is>(_current)...}; }   // via toValue(_seq)
```

The runtime ternary works because both branches have the same type
(`Component *` or `const Component *`). Note the extra parentheses required
around a binary expression inside a fold.

### Rules while iterating (each one is tested)

1. **Erasing the current entity is safe** (kill, `removeComponent`): in the
   driver, swap-and-pop brings in an element from the end, which was already visited.
2. **Erasing other entities is not supported** (an entity may be skipped or
   visited twice) but **stays memory-safe**: `++` clamps to the driver's current
   size. Collect such entities and kill them after the loop (or use deferred
   commands, step 12).
3. **Adding components** doesn't break the iterator (it holds the sets, not
   spans of their arrays; new entities aren't visited), but **component
   references from the loop dangle** after a reallocation.

### Performance

| GCC, small component, join with Position (ms) | Hand-written | First View | Final View |
|---|---|---|---|
| 1% | 0.088 | 0.211 | 0.116 |
| 10% | 1.48 | 2.36 | 1.66 |
| 50% | 2.20 | 3.16 | 2.44 |
| 100% | 2.93 | 5.48 | 3.27 |

The first version looked each set up twice (`contains` while skipping, `get` on
dereference) and looked up the driver through `_sparse`. Caching pointers from a
single `tryGet`, and reading the driver at its packed position, removed most of
that overhead **in this harness**.

### Benchmark: the view against the hand-written join (with a correction)

**Claim to verify:** the View costs about as much as the hand-written sparse-set
join from step 8. `IterationBenchmarks.cpp`, same setup as
`BM_Join_SparseSetManual` (step 8):

```cpp
template <class T>
void BM_Join_View(benchmark::State &state)
{
    // ... same setup as BM_Join_SparseSetManual ...
    for (auto _ : state) {
        float sum = 0.f;
        for (auto &&[pos, other] : ecs::View(positions, others))
            sum += pos.x * other.value;
        benchmark::DoNotOptimize(sum);
    }
    state.counters["matches"] = static_cast<double>(others.size());
}
BENCHMARK_TEMPLATE(BM_Join_View, Small) DENSITIES;
```

`compare.py filters $B 'BM_Join_SparseSetManual' 'BM_Join_View'`.

**What Google Benchmark showed.** The table above was measured with the
hand-rolled harness, built by CMake in `Release` mode, i.e. **`-O3`**. When
re-measured with Google Benchmark (medians over repetitions), the View's
overhead turned out to depend heavily on the **optimization level**:

| View / hand-written, small component | 1% | 50% | 100% |
|---|---|---|---|
| GCC `-O2`, Google Benchmark | 2.7× | 1.46× | 1.9× |
| GCC `-O3`, Google Benchmark | 1.48× | — | 1.11× |
| GCC `-O3`, hand-rolled harness | 1.55× | 1.05× | 1.11× |
| Clang `-O2`, Google Benchmark | 1.7× | — | 1.16× |

Changing `const` vs non-const sets, or inlining vs `noinline`, made no
difference; the optimization level did. At `-O2`, GCC doesn't inline the
View's runtime driver dispatch (the fold over every index on each step); at
`-O3` it does. **Corrected conclusion:** the View is about 1.1× the hand-written
join at medium and high density with `-O3`, up to about 1.5× at low density,
and about 2× with GCC `-O2`. The earlier "0–15% in most rows" was too
optimistic: it only holds for GCC `-O3` at medium to high density. (The
Clang `-O3` numbers we got were too noisy to report: cv 13–24% on a busy machine.)

**What this means for step 10:** the remaining cost is the **runtime driver
dispatch**. Holding the driver as a `const SparseSetBase *` removes it, and step
10's benchmark must check that on **both compilers and both `-O2` and `-O3`**.

**Lesson:** benchmark with the flags you actually ship, and when two harnesses
disagree, find out why before trusting either. Here, the difference was not
noise, but a real dependence on optimization level that the first harness
couldn't show.

### Demo: explicit draw order

Views have no meaningful order (backwards, smallest set), so "draw order = id
order" broke: the player ended up under the walls. Fix:
- `Drawable::layer` (`int layer = 0;`: the default member initializer keeps old
  initializations valid; the player uses layer 1);
- `DrawSystem` fills a member `std::vector<std::pair<const Position *, const
  Drawable *>> _queue` (`clear()` keeps capacity, so no allocation after the first
  frame), `std::ranges::stable_sort`s it by layer (stable: equal layers keep their
  order, so there's no flicker), then draws.

### Check yourself

Compile-time: reference types, `std::forward_iterator`, `forward_range<const
View<...>>`, CTAD. Runtime: only entities in every set; **order follows the
smallest set backwards**; writes go through; empty and disjoint sets; single and
three sets; const sets read-only; standard algorithms; post-increment; **killing
the current entity visits all entities exactly once**; removing the current
component; **killing every other entity from inside the loop doesn't crash
(ASan)**; adding entities during the loop: not visited, no crash.

---

## Testing strategy

Five layers, each catching different bugs:

1. **Unit tests** (GoogleTest, one file per class), written with each step.
2. **Compile-time tests**: `static_assert` with type traits and concepts for
   what must or must not compile (`!std::is_convertible_v<size_t, Entity>`,
   `std::forward_iterator<...>`, exact `reference` types, CTAD results).
3. **Sanitizers**: build regularly with
   `-fsanitize=address,undefined -fno-omit-frame-pointer`. ASan caught the
   `insertAt` aliasing (heap-use-after-free) and validates the "stays memory-safe"
   claims.
4. **Model-based fuzz test** (`tests/ECS/RegistryFuzzTests.cpp`): random
   spawn/kill/add/replace/remove/view-iteration (including kills of the current
   entity and writes through views), compared after many steps with a reference
   model made of `std::map`/`std::set`, which is slow but obviously correct. Fixed seeds
   (`INSTANTIATE_TEST_SUITE_P(Seeds, ..., Values(1u, 2u, 3u))`) make failures
   reproducible. A non-trivial component (`std::string`) makes swap-and-pop
   move heap memory. In the suite: 3 × 30,000 operations, about 2 s per seed under
   ASan; a longer offline run did 1.6 million operations with up to 34,000 live
   entities, with no difference and no sanitizer error.
5. **Benchmarks** (`benchmarks/`, see
   [Benchmarking with Google Benchmark](#benchmarking-with-google-benchmark)):
   a "Benchmark" section in each step whose design is about performance, always
   against a hand-written baseline, on both compilers, in Release, with
   repetitions. They found two things the tests couldn't: a 2× GCC-only cost in
   the bootstrap's zipper design (step 7), and the View's dependence on the
   optimization level (step 9).

Two lessons from the journey: when a test fails, check the test data as well as
the code (step 9's first run); and a test that is too slow stops being run
(the fuzz test was tuned from 30 s to 2 s per seed).

---

## Current rules and limitations

Rules to follow with the ECS as it is:

| Rule | Why | Removed by |
|---|---|---|
| Don't store `Entity` handles long-term | ids are recycled: a stored handle can designate a new entity (stale handle) | step 11 |
| Inside a View loop, only erase the **current** entity | erasing others can skip or repeat entities | step 12 (deferred commands); meanwhile collect and kill after the loop |
| Don't use component references after adding components | reallocation of the packed arrays | step 12; meanwhile copy the values first |
| Don't add components to dead entities | only an `assert` checks it: in Release it creates a ghost component | step 11 makes it detectable |
| Movement is per frame | frame-rate dependent | step 12 (delta time) |

Limitations:
- Components must be **copyable** (`std::any`) and **move-assignable**
  (swap-and-pop). A component holding a `std::unique_ptr` doesn't compile → step 10.
- Systems must be **copyable** callables (`std::function`).
- **Not thread-safe:** a network thread must not touch the registry; push
  messages to a queue and apply them in a system.
- Minor: `killEntity` can lose an id if `_freeIds.push_back` throws
  `bad_alloc`; `SparseSet::insertAt` returns the result of `operator=` directly,
  so a component whose assignment returns something unusual won't compile;
  `runSystems` re-entrancy isn't detected.

---

## Further improvements (steps 10+)

### Step 10: Registry v2, our own type erasure

**Goal:** remove `std::any`, `std::type_index` and the erasers; allow move-only
components; remove the View's runtime driver dispatch.

1. **Split the sparse set into an untyped base and a typed storage** (EnTT's
   `basic_sparse_set` / `basic_storage` design):
   ```cpp
   class SparseSetBase {
   public:
       using entity_type = std::uint32_t;            // also halves memory
       virtual ~SparseSetBase() = default;
       bool contains(entity_type e) const noexcept;
       std::size_t size() const noexcept;
       std::span<const entity_type> entities() const noexcept;
       virtual void erase(entity_type e) = 0;        // typed part moves the component
   protected:
       std::vector<std::uint32_t> _sparse;
       std::vector<entity_type> _dense;
   };

   template <class Component>
   class SparseSet final : public SparseSetBase {
       std::vector<Component> _components;
       void erase(entity_type e) override;           // swap-and-pop on both arrays
       ...
   };
   ```
   The entity half (`_sparse`, `_dense`, `contains`) doesn't depend on the
   component type.
2. **Integer type ids instead of `std::type_index`:**
   ```cpp
   inline std::size_t nextTypeId() noexcept
   {
       static std::atomic<std::size_t> counter{0};
       return counter++;
   }

   template <class T>
   std::size_t typeId() noexcept
   {
       static const std::size_t id = nextTypeId();   // initialized once per type, thread-safe
       return id;
   }
   ```
   The registry stores `std::vector<std::unique_ptr<SparseSetBase>> _storages`,
   indexed by `typeId<Component>()`, so a lookup is a vector index instead of a hash of
   a name. `getComponents<C>()` is `static_cast<SparseSet<C> &>(*_storages[typeId<C>()])`
   (safe: the index was computed from `C`, so no redundant runtime check).
   **Warnings:** ids depend on first-use order, so they differ between runs and
   between client and server: **never send them over the network** (use an
   explicit, ordered component list for networking). With shared libraries,
   make sure `typeId` isn't instantiated separately in each library.
3. **`killEntity` without erasers:** `for (auto &storage : _storages) if
   (storage) storage->erase(e);`, a virtual call per storage.
4. **Copying the registry:** `unique_ptr` makes it move-only. Either accept that
   (a game has one registry) or add `virtual std::unique_ptr<SparseSetBase>
   clone() const`, which is only available for copyable components.
5. **Move-only components** now work (no `std::any` copy requirement). Test with
   `struct Sprite { std::unique_ptr<Texture> texture; };`.
6. **View driver** = `const SparseSetBase *`: `driverEntity` and `driverSize`
   become direct calls, with no fold. Re-run the benchmark: the view should match
   the hand-written join.
7. Use the opportunity to fix the minor issues: assign then return the element
   in `insertAt`; `reserve` in `_freeIds` before marking the entity dead;
   optionally add a C++20 **concept** constraining components
   (`std::is_object_v<T> && std::movable<T>` and not cv-qualified) for clear errors.
8. Optionally store systems in `std::move_only_function` (C++23) or a small
   hand-written equivalent, to accept move-only callables.

**Benchmark plan for step 10.** Step 10 is a performance refactoring, so it is
validated as a **before/after comparison on the same benchmarks**: save
`before.json` from the last step 9 commit and `after.json` from step 10,
10 repetitions each, both compilers, at `-O3` and `-O2`, then run
`compare.py benchmarks before.json after.json`.

| Benchmark (introduced in) | Baseline measured | Expected after step 10 | Why |
|---|---|---|---|
| `BM_Registry_GetComponents` (step 3) | 33.8 ns | a few ns | vector index instead of hashing a mangled name; `static_cast` instead of `any_cast` |
| `BM_Registry_SpawnKill<1/8/32>` (step 4) | 142 / 502 / 1697 ns | still linear in types, but a much smaller slope (no `std::function`, no lookup per type) | virtual `erase` called directly on each storage |
| `BM_Registry_RunSystems` (step 6) | ~70 ns per system | lower (two cheap lookups per system) | integer type ids |
| `BM_Join_View` vs `BM_Join_SparseSetManual` (step 9) | 1.1–2.7× depending on compiler and flags | about 1.0–1.1× **on both compilers and both `-O2` and `-O3`** | driver held as `const SparseSetBase *`: no runtime dispatch fold |
| new `BM_SparseSet_Memory<T>` counter | 8 + 8 bytes per id/entity | 4 + 4 bytes | 32-bit indices |

**Acceptance criteria:** no benchmark more than 5% slower (with p < 0.05 in
`compare.py`), and the View within 10% of the hand-written join at every density
on both compilers. If the View doesn't get there, look at the generated code
(`objdump -d --no-show-raw-insn` or Compiler Explorer) before trying another
design.

### Step 11: Generational entities

**Goal:** detect stale handles.

- `Entity` = `{index : 32 bits, generation : 32 bits}` packed in a `std::uint64_t`.
- The registry keeps `std::vector<std::uint32_t> _generations` (one per index).
  `killEntity` increments the index's generation and pushes the index on the
  free list; `spawnEntity` reuses the index with its current generation.
- `isAlive(e)`: `e.index < _generations.size() && _generations[e.index] ==
  e.generation` (plus an alive flag, or use an odd/even generation scheme).
- Storages stay indexed by `index` (`_sparse[index]`), and `_dense` stores the
  full entity, so `contains(entity)` can check the generation too.
- A **null entity** constant (e.g. all bits set) replaces "no default
  constructor": `Entity target = Entity::null;`.
- `addComponent` on a dead entity can now throw in Release.
- Networking: send the full 64-bit id, so a client never confuses a dead
  server entity with a new one.
- Test: kill, respawn (same index), check that the old handle is not alive, that
  `get` through the old handle fails, and that a stored target is detected as dead.

**Benchmark plan for step 11.** Generations add a check to every validated
access, so the question is **how much safety costs**:

- `BM_Registry_IsAlive`: random handles, half stale. Expected: a few ns (one
  load and one compare). Same order as step 4's `_alive` lookup.
- `BM_Registry_SpawnKill<N>` before/after: the generation increment should not
  be measurable next to the per-type erase loop.
- `BM_Join_View` before/after: views iterate storages, which store full
  entities in `_dense`. If the View checks generations, expect at most a few %.
  If more, check generations only in handle-based APIs (`get(entity)`,
  `isAlive`), not inside views, whose entities are alive by construction.
- Memory counter: `_dense` stores 64-bit entities instead of 32-bit indices
  (+4 bytes per component). Report it as a counter so the cost is visible.

**Acceptance:** views and spawn/kill within 5% of step 10; `isAlive` under 5 ns.

### Step 12: Deferred commands, resources, delta time, scheduling

1. **Command buffer:** `registry.defer().kill(e)`, `.spawn(...)`,
   `.add<T>(e, value)`. Commands are queued (a vector of small structs, or of
   `std::move_only_function<void(Registry &)>`), and **applied after each system**
   (or at the end of the frame). This removes rules 2 and 3 of step 9: collisions
   can kill both entities, and enemies can spawn bullets from inside a View loop.
2. **Resources** (global singletons for systems):
   `registry.setResource<DeltaTime>(...)`, `getResource<T>()`, stored by
   `typeId<T>()` like components. Candidates: `DeltaTime`, `Input`, the window,
   the network inbox and outbox, the random generator.
3. **Delta time and a fixed timestep:** velocities in pixels per *second*;
   gameplay runs at a fixed rate (e.g. 60 Hz) with an accumulator:
   ```cpp
   accumulator += frameTime;
   while (accumulator >= step) { registry.runSystems(updateStage); accumulator -= step; }
   registry.runSystems(renderStage);
   ```
   For R-Type this keeps the server and clients deterministic, independent of
   frame rate.
4. **Stages / scheduling:** named groups of systems (input, fixed update,
   network, render). The server runs no render stage. Later, the read/write sets
   (const vs non-const parameters) let independent systems run in parallel.

**Benchmark plan for step 12.**

- **Deferred vs immediate destruction:** `BM_Kill_Immediate` (kill inside the
  loop, current entity only, the only safe case today) vs `BM_Kill_Deferred`
  (record in the command buffer, apply after the system), for 1%, 10% and 50% of
  entities dying per frame. Expected: deferred costs a small constant per
  command (an append to a vector), plus the same erase work later. Report
  `items_per_second` (commands). Deferred also enables what immediate can't
  (killing *other* entities), so the question is only "how much does the safety cost?".
- **Command representation:** small typed structs in a `std::vector` vs
  `std::move_only_function` per command. The second allocates for
  large captures; the benchmark decides whether that matters.
- **Resource lookup:** `BM_Registry_GetResource`: same mechanism as
  `getComponents` after step 10, so expect the same few ns.
- **Fixed timestep:** nothing to benchmark in isolation. Test determinism
  instead: the same inputs at 30 and 144 FPS must give the same positions.
- **Parallel stages** (if implemented): run independent systems on a thread pool
  and measure **real time** (`->UseRealTime()`) of a whole stage with 1, 2 and 4
  threads, on a realistic world (thousands of entities, 10+ systems). Expect
  speedup only when systems are long enough to pay for synchronization. Report
  the speedup per thread count, not just the absolute time.

### Other ideas

- **Paged `_sparse`** (pages of 4096 entries, allocated on demand): memory no
  longer proportional to the highest id.
- **Groups** (EnTT): keep several sets sorted in the same order so that
  full-density joins become plain parallel loops (fixes the 100%-density case
  where `SparseArray` still wins).
- **Component events** (`onConstruct<T>`, `onDestroy<T>` signals): useful for
  network replication (send what changed) and for resource cleanup.
- **Serialization**: snapshot/delta of selected components for the network
  protocol, with explicit stable component ids.
- **Remove `SparseArray` and `Zipper`** once the benchmark no longer needs them.
- **Migrate `benchmarks/` to Google Benchmark** as described in
  [Benchmarking with Google Benchmark](#benchmarking-with-google-benchmark),
  ideally **before step 10**, so step 10 has a clean "before" to compare with.

Benchmarks for these ideas:
- **Paged `_sparse`:** the `MB` counter of `BM_SparseSet_Iterate` at 0.1% density
  should drop from about 8 MB to a few hundred KB, and `BM_Join_View` must not get
  slower (page lookup = one shift + one mask).
- **Groups:** `BM_Join_Group` against `BM_Join_SparseArrayManual` at 100%
  density: the goal is to beat the 1.5× gap that `SparseArray` still has there
  (step 8).
- **Component events / serialization:** measure bytes per snapshot (a counter)
  and serialization time for 1k and 10k entities. The network budget (bytes per
  tick) matters more than CPU time here.

---

## Appendix: file map and C++ feature index

### Files

| File | Step | Role |
|---|---|---|
| `libs/ECS/include/Entity.hpp` | 1, 4 | strong id type, created only by `Registry` |
| `libs/ECS/include/SparseArray.hpp/.tpp` | 2 | `vector<optional<T>>` column (kept for benchmarks) |
| `libs/ECS/include/Registry.hpp/.tpp`, `libs/ECS/Registry.cpp` | 3, 4, 6, 9 | storages, entities, systems |
| `libs/ECS/include/ZipperIterator`, `Zipper`, `IndexedZipper` | 7 | iteration over `SparseArray`s |
| `libs/ECS/include/SparseSet.hpp/.tpp` | 8 | packed storage |
| `libs/ECS/include/ViewIterator`, `View`, `IndexedView` | 9 | iteration over `SparseSet`s |
| `src/Bootstrap/*` | 5, 6, 7, 9 | SFML demo |
| `tests/ECS/*Tests.cpp` | all | unit, compile-time and fuzz tests |
| `benchmarks/StorageBenchmark.cpp` | 8, 9 | array vs set vs view |

### C++ features and where they were introduced

| Feature | Step |
|---|---|
| `explicit` constructors, conversion operators, `constexpr`/`noexcept` | 1 |
| `static_assert` + type traits as compile-time tests | 1 |
| `std::optional`, rule of zero, `noexcept` moves and `move_if_noexcept` | 2 |
| Value categories, copy vs move overloads, aliasing on reallocation | 2 |
| Variadic templates, forwarding references, `std::forward`, `std::in_place` | 2 |
| `std::addressof`, `std::less` on pointers | 2 |
| `.tpp` files, out-of-class member templates, trailing return types | interlude |
| `typeid`, `std::type_index`, `std::unordered_map::try_emplace` | 3 |
| Type erasure: `void *`, base class, `std::any` and how it works | 3 |
| `friend`, forward declarations, `std::function`, strong exception guarantee | 4 |
| `std::remove_cvref_t` with forwarding references | 4 |
| Function objects, `std::as_const`, SFML 3 events (`std::optional` + `is`/`getIf`) | 5 |
| Parameter pack before a deduced parameter, pack expansion | 6 |
| Init-capture with `std::forward`, `mutable` lambdas | 6 |
| Fold expressions (`,` / `&&` / `||`), `std::is_invocable`, RAII guards | 6 |
| Iterator requirements and C++20 iterator concepts, proxy iterators | 7 |
| `decltype` + `std::declval`, `std::index_sequence`, `std::tuple`, `std::tuple_cat` | 7 |
| CTAD, structured bindings, `[[maybe_unused]]`, `std::initializer_list` from a pack | 7 |
| `std::span`, swap-and-pop, exception-safe multi-container updates | 8 |
| `std::variant`, `std::visit`, the overloaded-lambdas idiom | 8 |
| Runtime index → compile-time tuple element (fold + short-circuit), default member initializers, `std::ranges::stable_sort` | 9 |
