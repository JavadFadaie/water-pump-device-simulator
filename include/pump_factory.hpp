#ifndef PUMP_FACTORY_HPP
#define PUMP_FACTORY_HPP

#include "grundfos_pump.hpp"
#include "wilo_pump.hpp"

class pump_factory 
{
  public:
    pump_factory()
    {}

    void choose_pump_driver(const int & driver_id)
    {
        selected_pump_driver = create_pump_driver(driver_id);

        if(selected_pump_driver)
        {
            std::cout << "Pump driver created successfully with ID manufacturer " << driver_id << "  " << std::endl;
            selected_pump_driver->set_devices(); // Populate device_list
            selected_pump_driver->display_device(); // Display available devices
            std::string device_name;
            
            if (driver_id == static_cast<int>(driverId::PUMP_GRUNDFOS))
            {
                device_name = "Grundfos CRE 20"; // Valid device name
            }
            else
            {
                device_name = "Stratos MAXO 50"; // Valid device name
            }
            
            selected_pump_driver->set_simulation_duration(60); // Set simulation duration to 60 seconds
            selected_pump_driver->device_selection(device_name);
            selected_pump_driver->driver_simulation();
        }
        else
        {
            std::cout << "Failed to create pump driver." <<  std::endl;
        }
    }

    std::unique_ptr<driver_base> create_pump_driver(const int & driver_id)
    {
        if (driver_id == static_cast<int>(driverId::PUMP_GRUNDFOS)) 
        {
            return std::make_unique<grundfos_pump>();
        }
        else if (driver_id == static_cast<int>(driverId::PUMP_WILO)) 
        {
            return std::make_unique<wilo_pump>(); // Assuming wilo_pump is defined similarly
        }

        // Additional pump drivers can be added here
        return nullptr; // Return nullptr if no matching driver found
    }
    

  private:
    std::unique_ptr<driver_base> selected_pump_driver;


};

#endif 