#ifndef MODBUS_TYPES_HPP
#define MODBUS_TYPES_HPP

#include <cstdint>
#include <memory>

enum class RegisterType
{
    HoldingRegister,
    InputRegister
};

enum class AccessType
{
    ReadAccess,
    WriteAccess,
    ReadWriteAccess
};

struct ModbusRegister
{
    uint16_t address;
    uint16_t mValue;
    AccessType access;

    ModbusRegister(uint16_t addr, AccessType acc)
        : address(addr), mValue(0), access(acc)
    {}
};

constexpr uint16_t REG_FLOW_RATE   = 100;
constexpr uint16_t REG_PRESSURE    = 102;
constexpr uint16_t REG_PUMP_POWER  = 104;
constexpr uint16_t REG_WATER_LEVEL = 106;
constexpr uint16_t REG_RUN_TIME    = 108;
constexpr uint16_t REG_PUMP_ON     = 110;
constexpr uint16_t SENSOR_DATA_SIZE = 2;

#endif
