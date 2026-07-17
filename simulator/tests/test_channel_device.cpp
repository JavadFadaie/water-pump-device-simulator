#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "modbus_core.hpp"
#include "channel_device.hpp"

class ChannelDeviceTest : public ::testing::Test
{
protected:
    ModbusCore core;
    std::unique_ptr<channel_device<RegisterType, uint16_t>> ch;

    void SetUp() override
    {
        core.addRegisterBlock(RegisterType::InputRegister, 100, AccessType::ReadAccess, 20);
        ch = std::make_unique<channel_device<RegisterType, uint16_t>>(core);
    }
};

TEST_F(ChannelDeviceTest, AddAndGetChannelFloat)
{
    ch->addChannel("FLOW", RegisterType::InputRegister, 100);
    ch->setRegisterValueFloat("FLOW", 25.0f, 1.0f);
    EXPECT_NEAR(ch->getChannelValue("FLOW"), 25.0f, 0.001f);
}

TEST_F(ChannelDeviceTest, ScaleIsApplied)
{
    ch->addChannel("PRESSURE", RegisterType::InputRegister, 102);
    ch->setRegisterValueFloat("PRESSURE", 2.0f, 10.0f);
    EXPECT_NEAR(ch->getChannelValue("PRESSURE"), 20.0f, 0.001f);
}

TEST_F(ChannelDeviceTest, SetRegisterValueRaw)
{
    ch->addChannel("PUMP_ON", RegisterType::InputRegister, 110);
    ch->setRegisterValue("PUMP_ON", 1);
    EXPECT_NEAR(ch->getChannelValue("PUMP_ON"), 1.0f, 0.001f);
}

TEST_F(ChannelDeviceTest, UnknownChannelReturnsZero)
{
    EXPECT_EQ(ch->getChannelValue("NONEXISTENT"), 0.0f);
}

TEST_F(ChannelDeviceTest, SetOnUnknownChannelIsNoop)
{
    EXPECT_NO_THROW(ch->setRegisterValueFloat("GHOST", 99.0f, 1.0f));
    EXPECT_NO_THROW(ch->setRegisterValue("GHOST", 5));
}

TEST_F(ChannelDeviceTest, WritePropagatestoCore)
{
    ch->addChannel("FLOW", RegisterType::InputRegister, 100);
    ch->setRegisterValueFloat("FLOW", 15.5f, 1.0f);

    uint16_t hi = core.getRegisterValue(RegisterType::InputRegister, 100);
    uint16_t lo = core.getRegisterValue(RegisterType::InputRegister, 101);
    uint32_t bits = (static_cast<uint32_t>(hi) << 16) | lo;
    float decoded;
    std::memcpy(&decoded, &bits, sizeof(float));

    EXPECT_NEAR(decoded, 15.5f, 0.001f);
}

TEST_F(ChannelDeviceTest, RawWritePropagatestoCore)
{
    ch->addChannel("PUMP_ON", RegisterType::InputRegister, 110);
    ch->setRegisterValue("PUMP_ON", 1);
    EXPECT_EQ(core.getRegisterValue(RegisterType::InputRegister, 110), 1);
}

TEST_F(ChannelDeviceTest, GetChannelsReturnsAllAdded)
{
    ch->addChannel("A", RegisterType::InputRegister, 100);
    ch->addChannel("B", RegisterType::InputRegister, 102);
    ch->addChannel("C", RegisterType::InputRegister, 104);
    EXPECT_EQ(ch->getChannels().size(), 3u);
}

TEST_F(ChannelDeviceTest, MultipleChannelsIndependent)
{
    ch->addChannel("X", RegisterType::InputRegister, 100);
    ch->addChannel("Y", RegisterType::InputRegister, 104);
    ch->setRegisterValueFloat("X", 1.0f, 1.0f);
    ch->setRegisterValueFloat("Y", 2.0f, 1.0f);
    EXPECT_NEAR(ch->getChannelValue("X"), 1.0f, 0.001f);
    EXPECT_NEAR(ch->getChannelValue("Y"), 2.0f, 0.001f);
}
