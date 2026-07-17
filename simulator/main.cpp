#include <iostream>
#include "web_server.hpp"

int main()
{
    std::cout << "Water Pump Device Simulator" << std::endl;

    web_server server;
    server.start(8085, "./static");

    return 0;
}
