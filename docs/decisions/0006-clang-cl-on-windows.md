# ADR 0006: clang-cl is the Windows compiler

- **Status:** accepted
- **Date:** 2026-10-12

## Context

The first draft of the spec named MSVC (`cl`) as the Windows compiler, with `/W4 /WX /permissive-` and presets called `windows-msvc-*` on the "Visual Studio 17 2022" generator.

The CI was built differently: its Windows jobs ran on a Visual Studio 2026 image with the `ClangCL` toolset. The two drifted apart. The MSVC presets were never run by anyone, MSVC itself never compiled the code, and `-Wall -Wextra` was passed to every compiler. Under clang-cl, `-Wall` means `-Weverything`, which forced a list of `-Wno-…` workarounds and led to `-Werror` being removed.

## Decision

- **clang-cl is the only Windows compiler** for Part 1, on developer machines and in CI.
- The presets are `windows-clangcl-debug` and `windows-clangcl-release`. They set the `ClangCL` toolset and the x64 architecture, and name **no generator**, so CMake uses whichever Visual Studio is installed (2022 locally, 2026 on the CI runner).
- Warning flags are chosen by compiler front end, in `cmake/ProjectOptions.cmake`: `/W4 /WX` when `CMAKE_CXX_COMPILER_FRONTEND_VARIANT` is `MSVC` (clang-cl), `-Wall -Wextra -Werror` otherwise.
- CI builds and tests through these presets, the same ones developers use.

## Consequences

- **One compiler family on both platforms.** Linux Clang and clang-cl share a front end, so most diagnostics are the same and fewer errors appear only on Windows. Linux GCC remains the second opinion.
- **Never pass `-Wall` to clang-cl.** Use `/W4`, which is its equivalent of `-Wall -Wextra`.
- **MSVC `cl` is not supported and not tested.** Code that compiles here may need changes to build with it. Supporting it later means a third branch in `ProjectOptions.cmake`, its own presets and a CI job.
- **Windows developers need the clang-cl component** of Visual Studio ("C++ Clang tools for Windows"), in addition to the usual C++ workload.
- The link still uses the MSVC runtime and the Windows SDK, so `CMAKE_MSVC_RUNTIME_LIBRARY` and `gtest_force_shared_crt` stay as they are.
