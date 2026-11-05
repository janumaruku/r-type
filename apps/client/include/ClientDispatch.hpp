/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientDispatch
*/

#ifndef R_TYPE_CLIENT_DISPATCH_HPP
#define R_TYPE_CLIENT_DISPATCH_HPP

#include <map>

#include "engine/core/ICommand.hpp"
#include "ServerLink.hpp"
#include "engine/core/FactoryTemplate.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::client {
class ConnectingScene;

class ClientLogLimiter {
public:
    [[nodiscard]] bool shouldLog(protocol::DecodeError code, TimePoint now);

private:
    std::map<protocol::DecodeError, TimePoint> _last;
};

struct ReceivedMessage {
    protocol::Header header;
    net::Packet packet;
};

[[nodiscard]] std::optional<ReceivedMessage> checkFromServer(
    const net::Datagram &datagram,
    const ServerLink &link, ClientLogLimiter &logs, TimePoint now);

using IConnectCommand = designPattern::ICommand<ConnectingScene &, net::Packet &>;
using ConnectCommandFactory = designPattern::FactoryTemplate<IConnectCommand,
    protocol::MessageType>;

}

#endif //R_TYPE_CLIENT_DISPATCH_HPP
