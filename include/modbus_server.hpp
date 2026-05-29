#ifndef MODBUS_SERVER_HPP
#define MODBUS_SERVER_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include "modbus_core.hpp"

class ModbusServer
{
public:
    ModbusServer(uint8_t slaveId = 1)
    {
        mModbusCore = std::make_shared<ModbusCore>();
        mSlaveId.push_back(slaveId);
    }

    virtual ~ModbusServer() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    std::shared_ptr<ModbusCore> mModbusCore;

protected:
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    std::vector<uint8_t> mSlaveId;
};

class ModbusTcp : public ModbusServer
{
public:
    ModbusTcp(uint8_t slaveId = 1,
              const std::string& ip = "0.0.0.0",
              int port = 1502);
    ~ModbusTcp() override;

    void start() override;
    void stop() override;

    void initModbusRegisters();

private:
    std::string ip_;
    int port_;
    void serverLoop();
};

class ModbusRtu : public ModbusServer
{
public:
    ModbusRtu(uint8_t slaveId = 1,
              const std::string& device = "/dev/ttyUSB0",
              int baud = 9600);
    ~ModbusRtu() override;

    void start() override;
    void stop() override;

private:
    std::string device_;
    int baud_;
};

#endif
