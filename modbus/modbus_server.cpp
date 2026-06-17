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

void ModbusTcp::start()
{
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
    constexpr int HOLDING_REG_COUNT = 512;
    constexpr int INPUT_REG_COUNT = 512;

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

// --- ModbusRtu ---

ModbusRtu::ModbusRtu(uint8_t slaveId, const std::string& device, int baud)
    : ModbusServer(slaveId), device_(device), baud_(baud)
{
}

ModbusRtu::ModbusRtu(uint8_t slaveId, std::shared_ptr<ModbusCore> core,
                     const std::string& device, int baud)
    : ModbusServer(slaveId, core), device_(device), baud_(baud)
{
}

ModbusRtu::~ModbusRtu()
{
    stop();
}

void ModbusRtu::start()
{
    running_ = true;
    server_thread_ = std::thread(&ModbusRtu::serverLoop, this);
}

void ModbusRtu::stop()
{
    running_ = false;
    if (server_thread_.joinable())
    {
        server_thread_.join();
    }
}

void ModbusRtu::serverLoop()
{
    constexpr int HOLDING_REG_COUNT = 512;
    constexpr int INPUT_REG_COUNT = 512;

    modbus_t* ctx = modbus_new_rtu(device_.c_str(), baud_, 'N', 8, 1);
    if (!ctx)
    {
        std::cerr << "Failed to create Modbus RTU context for " << device_ << std::endl;
        return;
    }

    modbus_set_slave(ctx, mSlaveId[0]);

    if (modbus_connect(ctx) == -1)
    {
        std::cerr << "Failed to open serial port " << device_ << ": " << modbus_strerror(errno) << std::endl;
        modbus_free(ctx);
        return;
    }

    modbus_mapping_t* mb_mapping = modbus_mapping_new(0, 0, HOLDING_REG_COUNT, INPUT_REG_COUNT);
    if (!mb_mapping)
    {
        std::cerr << "Failed to allocate Modbus mapping for RTU" << std::endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return;
    }

    int fd = modbus_get_socket(ctx);
    std::cout << "Modbus RTU server started on " << device_ << " at " << baud_ << " baud (slave ID " << (int)mSlaveId[0] << ")" << std::endl;

    uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];

    while (running_)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int rc = select(fd + 1, &rfds, nullptr, nullptr, &tv);
        if (rc < 0)
        {
            break;
        }
        if (rc == 0)
        {
            continue;
        }

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
            if (errno != ETIMEDOUT)
            {
                std::cerr << "Modbus RTU receive error on " << device_ << ": "
                          << modbus_strerror(errno) << std::endl;
                modbus_flush(ctx);
            }
        }
    }

    modbus_close(ctx);
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);

    std::cout << "Modbus RTU server stopped on " << device_ << std::endl;
}
