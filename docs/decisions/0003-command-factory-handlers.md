# ADR 0003: One command class per message type, created by a factory

- **Status:** accepted
- **Date:** 2026-10-05

## Context

The server and the client each receive a handful of message types (3 on the server, 5 on the client), and the list will grow in Part 2. Each type needs its own decoding and handling. The team already used a Command + Factory pattern for the Zappy protocol handlers (`ICommand`, `FactoryTemplate`, one class per command, `registerCreators()`), and knows it well.

One variant of factory registration is avoided: a macro that registers a creator as a side effect of a static variable in the command's `.cpp` file. Our modules are static libraries, and the linker drops any object file of a static library that nothing references. A self-registered command is referenced by nobody, so its registration would silently never run.

## Decision

- `engine_core` provides the generic `ICommand<Args...>` and `FactoryTemplate<Base, Key, Args...>` templates, as in Zappy.
- Each message type that a side receives has one command class with `execute`, `operator()` and a static `create()`.
- The class that receives the messages (`ServerNetworkAdapter`, `ConnectingScene`, `ClientNetworkAdapter`) owns a factory keyed by `protocol::MessageType` and fills it in a `registerCommands()` method called from its constructor:

```cpp
void ServerNetworkAdapter::registerCommands() noexcept
{
    _commands.registerCreator<HelloCommand>(protocol::MessageType::HELLO);
    _commands.registerCreator<InputCommand>(protocol::MessageType::INPUT);
    _commands.registerCreator<DisconnectCommand>(protocol::MessageType::DISCONNECT);
}
```

- The decode boundary checks `hasKey(type)` before `create(type)`, so an unknown or wrong-direction type from the network counts as an offence without throwing.

## Consequences

- Each message's handling lives in its own small class and file, so teammates can work on different messages without touching the same code.
- Adding a message type means one new command class and one `registerCreator` line, and `registerCommands()` lists every supported message in one place.
- `registerCreator<T>` only compiles when `T::create()` returns the right type, so a wrongly shaped command is a compile error.
- `registerCommands()` includes every command's header, so the linker always keeps the command code.
- `FactoryTemplate::create` allocates one command object per message: about 240 small allocations per second on the server at full load, which is negligible.
- `protocol::MessageType` needs an `operator<<`, because `FactoryTemplate::create` uses it to build its error message.
