#ifndef MODBUS_CORE_HPP
#define MODBUS_CORE_HPP

#include "modbus_types.hpp"
#include "protocol_writer.hpp"
#include <map>
#include <mutex>
#include <cstring>
#include <vector>
#include <iostream>

class ModbusCore : public ProtocolWriter<RegisterType, uint16_t>
{
public:
    void addRegisterBlock(RegisterType type, uint16_t startAddr, AccessType access, uint16_t count) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto& regMap = getRegMap(type);
        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t addr = startAddr + i;
            if (regMap.find(addr) == regMap.end())
            {
                regMap.emplace(addr, std::make_shared<ModbusRegister>(addr, access));
            }
        }
    }

    void setRegisterValue(RegisterType type, uint16_t addr, uint16_t value) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto& regMap = getRegMap(type);
        auto it = regMap.find(addr);
        if (it != regMap.end())
        {
            it->second->mValue = value;
        }
    }

    void setRegisterValueFloat(RegisterType type, uint16_t addr, float value) override
    {
        uint32_t raw;
        std::memcpy(&raw, &value, sizeof(float));
        uint16_t hi = static_cast<uint16_t>(raw >> 16);
        uint16_t lo = static_cast<uint16_t>(raw & 0xFFFF);

        std::lock_guard<std::mutex> lock(mMutex);
        auto& regMap = getRegMap(type);

        auto it_hi = regMap.find(addr);
        auto it_lo = regMap.find(addr + 1);
        if (it_hi != regMap.end()) it_hi->second->mValue = hi;
        if (it_lo != regMap.end()) it_lo->second->mValue = lo;
    }

    uint16_t getRegisterValue(RegisterType type, uint16_t addr) const override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        const auto& regMap = getRegMapConst(type);
        auto it = regMap.find(addr);
        if (it != regMap.end())
        {
            return it->second->mValue;
        }
        return 0;
    }

    std::vector<uint16_t> readRegisters(RegisterType type, uint16_t startAddr, uint16_t count) const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        const auto& regMap = getRegMapConst(type);
        std::vector<uint16_t> result(count, 0);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto it = regMap.find(startAddr + i);
            if (it != regMap.end())
            {
                result[i] = it->second->mValue;
            }
        }
        return result;
    }

    void syncToMapping(uint16_t* holdingRegs, int holdingSize,
                       uint16_t* inputRegs, int inputSize) const
    {
        std::lock_guard<std::mutex> lock(mMutex);

        for (const auto& [addr, reg] : mHoldingRegisters)
        {
            if (addr < holdingSize)
            {
                holdingRegs[addr] = reg->mValue;
            }
        }

        for (const auto& [addr, reg] : mInputRegisters)
        {
            if (addr < inputSize)
            {
                inputRegs[addr] = reg->mValue;
            }
        }
    }

private:
    std::map<uint16_t, std::shared_ptr<ModbusRegister>> mHoldingRegisters;
    std::map<uint16_t, std::shared_ptr<ModbusRegister>> mInputRegisters;
    mutable std::mutex mMutex;

    std::map<uint16_t, std::shared_ptr<ModbusRegister>>& getRegMap(RegisterType type)
    {
        return (type == RegisterType::HoldingRegister) ? mHoldingRegisters : mInputRegisters;
    }

    const std::map<uint16_t, std::shared_ptr<ModbusRegister>>& getRegMapConst(RegisterType type) const
    {
        return (type == RegisterType::HoldingRegister) ? mHoldingRegisters : mInputRegisters;
    }
};

#endif
