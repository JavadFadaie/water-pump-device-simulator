#include <gtest/gtest.h>
#include <cstring>
#include "modbus_core.hpp"

TEST(ModbusCoreTest, AddAndGetHoldingRegister)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 100, AccessType::ReadAccess, 2);
    core.setRegisterValue(RegisterType::HoldingRegister, 100, 42);
    EXPECT_EQ(core.getRegisterValue(RegisterType::HoldingRegister, 100), 42);
}

TEST(ModbusCoreTest, AddAndGetInputRegister)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::InputRegister, 200, AccessType::ReadAccess, 1);
    core.setRegisterValue(RegisterType::InputRegister, 200, 1234);
    EXPECT_EQ(core.getRegisterValue(RegisterType::InputRegister, 200), 1234);
}

TEST(ModbusCoreTest, FloatEncodingDecoding)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 100, AccessType::ReadAccess, 2);
    core.setRegisterValueFloat(RegisterType::HoldingRegister, 100, 3.14f);

    uint16_t hi = core.getRegisterValue(RegisterType::HoldingRegister, 100);
    uint16_t lo = core.getRegisterValue(RegisterType::HoldingRegister, 101);
    uint32_t bits = (static_cast<uint32_t>(hi) << 16) | lo;
    float result;
    std::memcpy(&result, &bits, sizeof(float));

    EXPECT_NEAR(result, 3.14f, 0.001f);
}

TEST(ModbusCoreTest, FloatZeroEncoding)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 0, AccessType::ReadAccess, 2);
    core.setRegisterValueFloat(RegisterType::HoldingRegister, 0, 0.0f);
    EXPECT_EQ(core.getRegisterValue(RegisterType::HoldingRegister, 0), 0);
    EXPECT_EQ(core.getRegisterValue(RegisterType::HoldingRegister, 1), 0);
}

TEST(ModbusCoreTest, UnregisteredAddressReturnsZero)
{
    ModbusCore core;
    EXPECT_EQ(core.getRegisterValue(RegisterType::HoldingRegister, 999), 0);
    EXPECT_EQ(core.getRegisterValue(RegisterType::InputRegister, 999), 0);
}

TEST(ModbusCoreTest, ReadRegisters)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 100, AccessType::ReadAccess, 3);
    core.setRegisterValue(RegisterType::HoldingRegister, 100, 10);
    core.setRegisterValue(RegisterType::HoldingRegister, 101, 20);
    core.setRegisterValue(RegisterType::HoldingRegister, 102, 30);

    auto regs = core.readRegisters(RegisterType::HoldingRegister, 100, 3);
    ASSERT_EQ(regs.size(), 3u);
    EXPECT_EQ(regs[0], 10);
    EXPECT_EQ(regs[1], 20);
    EXPECT_EQ(regs[2], 30);
}

TEST(ModbusCoreTest, ReadRegistersUnknownAddressReturnsZero)
{
    ModbusCore core;
    auto regs = core.readRegisters(RegisterType::HoldingRegister, 500, 3);
    ASSERT_EQ(regs.size(), 3u);
    EXPECT_EQ(regs[0], 0);
    EXPECT_EQ(regs[1], 0);
    EXPECT_EQ(regs[2], 0);
}

TEST(ModbusCoreTest, SyncToMappingHolding)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 0, AccessType::ReadAccess, 3);
    core.setRegisterValue(RegisterType::HoldingRegister, 0, 11);
    core.setRegisterValue(RegisterType::HoldingRegister, 1, 22);
    core.setRegisterValue(RegisterType::HoldingRegister, 2, 33);

    uint16_t holding[512] = {};
    uint16_t input[512]   = {};
    core.syncToMapping(holding, 512, input, 512);

    EXPECT_EQ(holding[0], 11);
    EXPECT_EQ(holding[1], 22);
    EXPECT_EQ(holding[2], 33);
}

TEST(ModbusCoreTest, SyncToMappingInput)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::InputRegister, 0, AccessType::ReadAccess, 2);
    core.setRegisterValue(RegisterType::InputRegister, 0, 77);
    core.setRegisterValue(RegisterType::InputRegister, 1, 88);

    uint16_t holding[512] = {};
    uint16_t input[512]   = {};
    core.syncToMapping(holding, 512, input, 512);

    EXPECT_EQ(input[0], 77);
    EXPECT_EQ(input[1], 88);
}

TEST(ModbusCoreTest, OverwriteRegisterValue)
{
    ModbusCore core;
    core.addRegisterBlock(RegisterType::HoldingRegister, 10, AccessType::ReadAccess, 1);
    core.setRegisterValue(RegisterType::HoldingRegister, 10, 100);
    core.setRegisterValue(RegisterType::HoldingRegister, 10, 200);
    EXPECT_EQ(core.getRegisterValue(RegisterType::HoldingRegister, 10), 200);
}
