#ifndef PUMP_FACTORY_HPP
#define PUMP_FACTORY_HPP

#include "driver_registry.hpp"
#include "driver_base.hpp"

class pump_factory
{
  public:
    pump_factory() = default;

    std::unique_ptr<driver_base> create_pump_driver(int driver_id)
    {
        return driver_registry::instance().create_driver(driver_id);
    }

    std::vector<std::pair<int, std::string>> list_available_drivers() const
    {
        return driver_registry::instance().list_drivers();
    }

  private:
    std::unique_ptr<driver_base> selected_pump_driver;
};

#endif
