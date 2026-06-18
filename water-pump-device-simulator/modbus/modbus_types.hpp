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

#endif
