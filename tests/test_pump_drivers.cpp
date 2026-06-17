#include <gtest/gtest.h>
#include "grundfos_pump.hpp"
#include "wilo_pump.hpp"
#include "model_info.hpp"

// ---- Grundfos ----

TEST(GrundfosDriverTest, AddModelNames)
{
    std::vector<ModelInfo> models;
    grundfos_pump::add_model(models);
    ASSERT_EQ(models.size(), 2u);
    EXPECT_EQ(models[0].Name, "Grundfos CRE 20");
    EXPECT_EQ(models[1].Name, "Grundfos CRE 30");
}

TEST(GrundfosDriverTest, AddModelIds)
{
    std::vector<ModelInfo> models;
    grundfos_pump::add_model(models);
    EXPECT_EQ(models[0].deviceId, 0);
    EXPECT_EQ(models[1].deviceId, 1);
}

TEST(GrundfosDriverTest, ModelProtocols)
{
    std::vector<ModelInfo> models;
    grundfos_pump::add_model(models);

    EXPECT_TRUE(models[0].SupportEth);
    EXPECT_FALSE(models[0].SupportRS485);
    EXPECT_EQ(models[0].EthProtocol, CommProtocol::PROTOCOL_MODBUS_TCP);

    EXPECT_TRUE(models[1].SupportEth);
    EXPECT_TRUE(models[1].SupportRS485);
    EXPECT_EQ(models[1].EthProtocol,    CommProtocol::PROTOCOL_MODBUS_TCP);
    EXPECT_EQ(models[1].SerialProtocol, CommProtocol::PROTOCOL_MODBUS_RTU);
}

TEST(GrundfosDriverTest, ConstructionSucceeds)
{
    EXPECT_NO_THROW({ grundfos_pump pump; });
}

TEST(GrundfosDriverTest, SetDevicesPopulatesDeviceList)
{
    grundfos_pump pump;
    pump.set_devices();
    EXPECT_EQ(pump.get_device_list().size(), 2u);
}

TEST(GrundfosDriverTest, SelectDeviceByIndexValid)
{
    grundfos_pump pump;
    pump.set_devices();
    EXPECT_TRUE(pump.select_device_by_index(0));
    EXPECT_TRUE(pump.select_device_by_index(1));
}

TEST(GrundfosDriverTest, SelectDeviceByIndexOutOfRange)
{
    grundfos_pump pump;
    pump.set_devices();
    EXPECT_FALSE(pump.select_device_by_index(-1));
    EXPECT_FALSE(pump.select_device_by_index(2));
}

TEST(GrundfosDriverTest, NewInstanceFactory)
{
    auto instance = grundfos_pump::new_instance();
    EXPECT_NE(instance, nullptr);
}

TEST(GrundfosDriverTest, NotRunningAfterConstruction)
{
    grundfos_pump pump;
    EXPECT_FALSE(pump.is_running());
}

// ---- Wilo ----

TEST(WiloDriverTest, AddModelNames)
{
    std::vector<ModelInfo> models;
    wilo_pump::add_model(models);
    ASSERT_EQ(models.size(), 2u);
    EXPECT_EQ(models[0].Name, "Stratos MAXO 50");
    EXPECT_EQ(models[1].Name, "Stratos MAXO 60");
}

TEST(WiloDriverTest, AddModelIds)
{
    std::vector<ModelInfo> models;
    wilo_pump::add_model(models);
    EXPECT_EQ(models[0].deviceId, 0);
    EXPECT_EQ(models[1].deviceId, 1);
}

TEST(WiloDriverTest, ModelProtocols)
{
    std::vector<ModelInfo> models;
    wilo_pump::add_model(models);

    EXPECT_TRUE(models[0].SupportEth);
    EXPECT_TRUE(models[0].SupportRS485);
    EXPECT_EQ(models[0].EthProtocol,    CommProtocol::PROTOCOL_MODBUS_TCP);
    EXPECT_EQ(models[0].SerialProtocol, CommProtocol::PROTOCOL_MODBUS_RTU);

    EXPECT_TRUE(models[1].SupportEth);
    EXPECT_TRUE(models[1].SupportRS485);
}

TEST(WiloDriverTest, ConstructionSucceeds)
{
    EXPECT_NO_THROW({ wilo_pump pump; });
}

TEST(WiloDriverTest, SetDevicesPopulatesDeviceList)
{
    wilo_pump pump;
    pump.set_devices();
    EXPECT_EQ(pump.get_device_list().size(), 2u);
}

TEST(WiloDriverTest, SelectDeviceByIndexValid)
{
    wilo_pump pump;
    pump.set_devices();
    EXPECT_TRUE(pump.select_device_by_index(0));
    EXPECT_TRUE(pump.select_device_by_index(1));
}

TEST(WiloDriverTest, SelectDeviceByIndexOutOfRange)
{
    wilo_pump pump;
    pump.set_devices();
    EXPECT_FALSE(pump.select_device_by_index(-1));
    EXPECT_FALSE(pump.select_device_by_index(2));
}

TEST(WiloDriverTest, NewInstanceFactory)
{
    auto instance = wilo_pump::new_instance();
    EXPECT_NE(instance, nullptr);
}

TEST(WiloDriverTest, NotRunningAfterConstruction)
{
    wilo_pump pump;
    EXPECT_FALSE(pump.is_running());
}
