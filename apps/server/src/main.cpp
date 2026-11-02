#include <iostream>

#include "ServerConfig.hpp"

int main(int argc, char *argv[])
{
    rtype::server::ServerConfig config;
    return parse(argc, argv, config);
}
