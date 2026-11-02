#include <iostream>

#include "ServerConfig.hpp"
#include "Conversions.hpp"

int main(int argc, char *argv[])
{
    rtype::server::ServerConfig config;
    parse(argc, argv, config);
    (void)rtype::server::toProtocol(rtype::game::EntityKind::PLAYER_SHIP);
}
