# ADR 0001: Use standalone Asio directly, with no wrapper

- **Status:** accepted
- **Date:** 2026-10-05

## Context

The subject allows Asio or OS sockets, and requires raw OS sockets to be wrapped in proper abstractions (page 9). The first draft of the spec wrapped Asio too, in `net::Endpoint` and `net::UdpTransport`, to keep `asio::` out of public headers. For a small team, that meant more code to write and more types to learn, and it protected against nothing likely: we are not going to replace Asio during the project.

## Decision

- Use **standalone Asio 1.30.2** (the `asio::` namespace, header-only), fetched through CPM.cmake. Boost.Asio has the same API but would pull in Boost through vcpkg or Conan.
- Use Asio types directly: `asio::io_context`, `asio::ip::udp::socket`, `asio::ip::udp::endpoint`, `asio::ip::udp::resolver`.
- The `network` target adds only two things of its own: `net::Packet` for safe serialization, and `net::UdpSocket`, which runs the socket on its own thread with a bounded receive queue. Both are described on the [network page](../modules/network.md).
- `rtype_game` and `rtype_protocol` never include Asio.

## Standalone Asio or Boost.Asio?

They are the same library. Boost.Asio is generated from standalone Asio by the same author, so the networking features are identical:

| | Standalone Asio | Boost.Asio |
| --- | --- | --- |
| Namespace and include | `asio::`, `#include <asio.hpp>` | `boost::asio::`, `#include <boost/asio.hpp>` |
| Error type | `std::error_code` | `boost::system::error_code` |
| Dependencies | None, header-only | Boost.System, Boost.Config and other Boost headers |
| Getting it with a package manager | A small download, fetched by CPM in seconds | Boost through vcpkg or Conan: much larger and slower to build, especially in CI |
| Extras | — | Works with other Boost libraries (Beast for HTTP/WebSocket, Boost.Context coroutines) |

We need none of the Boost extras, the team already used standalone Asio in Zappy and EPIJAM, and the subject warns against heavy dependency downloads in CI. Standalone Asio is the better fit. Switching later would mean renaming `asio::` to `boost::asio::` in the network code only.

## Consequences

- Asio already is the abstraction the subject asks for, so we satisfy T-07 without writing one.
- Asio's documentation and examples apply to our code as they are.
- Asio types appear in the server and client network classes. Moving away from Asio later would mean touching those classes, but not the game or the protocol.
