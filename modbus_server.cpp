#include "modbus_server.hpp"
#include <modbus/modbus.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

// --- ModbusTcp ---

ModbusTcp::ModbusTcp(uint8_t slaveId, const std::string& ip, int port)
    : ModbusServer(slaveId), ip_(ip), port_(port)
{
}

ModbusTcp::~ModbusTcp()
{
    stop();
}

void ModbusTcp::initModbusRegisters()
{
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_FLOW_RATE, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PRESSURE, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PUMP_POWER, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_WATER_LEVEL, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_RUN_TIME, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PUMP_ON, AccessType::ReadAccess, 1);

    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_FLOW_RATE, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_PRESSURE, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_PUMP_POWER, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_WATER_LEVEL, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_RUN_TIME, AccessType::ReadAccess, SENSOR_DATA_SIZE);
    mModbusCore->addRegisterBlock(RegisterType::HoldingRegister, REG_PUMP_ON, AccessType::ReadAccess, 1);
}

void ModbusTcp::start()
{
    initModbusRegisters();
    running_ = true;
    server_thread_ = std::thread(&ModbusTcp::serverLoop, this);
}

void ModbusTcp::stop()
{
    running_ = false;
    if (server_thread_.joinable())
    {
        server_thread_.join();
    }
}

void ModbusTcp::serverLoop()
{
    constexpr int HOLDING_REG_COUNT = 214;
    constexpr int INPUT_REG_COUNT = 112;

    modbus_t* ctx = modbus_new_tcp(ip_.c_str(), port_);
    if (!ctx)
    {
        std::cerr << "Failed to create Modbus TCP context" << std::endl;
        return;
    }

    modbus_set_slave(ctx, mSlaveId[0]);

    modbus_mapping_t* mb_mapping = modbus_mapping_new(0, 0, HOLDING_REG_COUNT, INPUT_REG_COUNT);
    if (!mb_mapping)
    {
        std::cerr << "Failed to allocate Modbus mapping" << std::endl;
        modbus_free(ctx);
        return;
    }

    int server_socket = modbus_tcp_listen(ctx, 5);
    if (server_socket < 0)
    {
        std::cerr << "Failed to listen on port " << port_ << std::endl;
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return;
    }

    std::cout << "Modbus TCP server listening on " << ip_ << ":" << port_ << std::endl;

    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

    while (running_)
    {
        fd_set refset;
        FD_ZERO(&refset);
        FD_SET(server_socket, &refset);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int rc = select(server_socket + 1, &refset, nullptr, nullptr, &tv);
        if (rc < 0)
        {
            break;
        }
        if (rc == 0)
        {
            continue;
        }

        modbus_tcp_accept(ctx, &server_socket);

        bool client_connected = true;
        while (running_ && client_connected)
        {
            rc = modbus_receive(ctx, query);
            if (rc > 0)
            {
                mModbusCore->syncToMapping(
                    mb_mapping->tab_registers, HOLDING_REG_COUNT,
                    mb_mapping->tab_input_registers, INPUT_REG_COUNT);

                modbus_reply(ctx, query, rc, mb_mapping);
            }
            else if (rc == -1)
            {
                client_connected = false;
            }
        }
    }

    close(server_socket);
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);
}

// --- ModbusRtu (stub) ---

ModbusRtu::ModbusRtu(uint8_t slaveId, const std::string& device, int baud)
    : ModbusServer(slaveId), device_(device), baud_(baud)
{
}

ModbusRtu::~ModbusRtu()
{
    stop();
}

void ModbusRtu::start()
{
    std::cout << "ModbusRtu: RTU transport not yet implemented" << std::endl;
}

void ModbusRtu::stop()
{
}
