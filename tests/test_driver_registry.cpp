#include <gtest/gtest.h>
#include "driver_registry.hpp"
#include "grundfos_pump.hpp"
#include "wilo_pump.hpp"

TEST(DriverRegistryTest, SingletonIsConsistent)
{
    EXPECT_EQ(&driver_registry::instance(), &driver_registry::instance());
}

TEST(DriverRegistryTest, BothDriversRegistered)
{
    auto drivers = driver_registry::instance().list_drivers();
    EXPECT_GE(drivers.size(), 2u);
}

TEST(DriverRegistryTest, CreateGrundfosDriver)
{
    auto driver = driver_registry::instance().create_driver(
        static_cast<int>(DriverId::GRUNDFOS_PUMP));
    EXPECT_NE(driver, nullptr);
}

TEST(DriverRegistryTest, CreateWiloDriver)
{
    auto driver = driver_registry::instance().create_driver(
        static_cast<int>(DriverId::WILO_PUMP));
    EXPECT_NE(driver, nullptr);
}

TEST(DriverRegistryTest, CreateUnknownDriverReturnsNull)
{
    auto driver = driver_registry::instance().create_driver(999);
    EXPECT_EQ(driver, nullptr);
}

TEST(DriverRegistryTest, GrundfosModelCount)
{
    auto models = driver_registry::instance().get_models(
        static_cast<int>(DriverId::GRUNDFOS_PUMP));
    EXPECT_EQ(models.size(), 2u);
}

TEST(DriverRegistryTest, WiloModelCount)
{
    auto models = driver_registry::instance().get_models(
        static_cast<int>(DriverId::WILO_PUMP));
    EXPECT_EQ(models.size(), 2u);
}

TEST(DriverRegistryTest, GetModelsUnknownIdReturnsEmpty)
{
    auto models = driver_registry::instance().get_models(999);
    EXPECT_TRUE(models.empty());
}

TEST(DriverRegistryTest, FindGrundfosDescriptor)
{
    auto desc = driver_registry::instance().find_descriptor(
        static_cast<int>(DriverId::GRUNDFOS_PUMP));
    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc->name, "Grundfos");
}

TEST(DriverRegistryTest, FindWiloDescriptor)
{
    auto desc = driver_registry::instance().find_descriptor(
        static_cast<int>(DriverId::WILO_PUMP));
    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc->name, "Wilo");
}

TEST(DriverRegistryTest, FindUnknownDescriptorReturnsNull)
{
    EXPECT_EQ(driver_registry::instance().find_descriptor(999), nullptr);
}

TEST(DriverRegistryTest, ListDriversContainsExpectedNames)
{
    auto drivers = driver_registry::instance().list_drivers();
    bool found_grundfos = false;
    bool found_wilo     = false;
    for (const auto& [id, name] : drivers)
    {
        if (name == "Grundfos") found_grundfos = true;
        if (name == "Wilo")     found_wilo     = true;
    }
    EXPECT_TRUE(found_grundfos);
    EXPECT_TRUE(found_wilo);
}
