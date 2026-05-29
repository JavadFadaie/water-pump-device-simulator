#include <iostream>
#include "web_server.hpp"
#include "modbus_server.hpp"

int main()
{
    std::cout << "Water Pump Device Simulator" << std::endl;

    auto modbus = std::make_unique<ModbusTcp>(1, "0.0.0.0", 1502);
    modbus->start();

    web_server server;
    server.setModbusCore(modbus->mModbusCore);
    server.start(8085, "./static");

    modbus->stop();

    return 0;
}
