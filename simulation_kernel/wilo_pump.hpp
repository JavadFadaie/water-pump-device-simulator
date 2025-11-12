#ifndef WILO_PUMP_HPP
#define WILO_PUMP_HPP

#include "driver_base.hpp"
#include "driver_registery.hpp"
#include "driverId.hpp"

class wilo_pump: public driver_base
{
  public: 
    wilo_pump()
    :driver_base(wilo_simulation_pump)
    {}

    void driver_init_info()
    {
      driver_info = driver_registery(static_cast<int>(driverId::PUMP_WILO), "Wilo Pump Driver", "Wilo");
    }
    
    void set_devices() override
    {
        device_list = 
        { 
	          { .device_name = "Stratos MAXO 50", .max_flow_rate=50.0, .max_pressure = 2, .power = 500}, 
		        { .device_name = "Stratos MAXO 60", .max_flow_rate=60.0, .max_pressure = 3, .power = 600}
        };
    }

    void set_simulation_duration(int duration_seconds)
    {
        simulation_duration = duration_seconds;
    }

    void driver_simulation()
    {
        generate_simulation();
    }

    void update_driver_value() override
    {
       std::cout << "[Flow Rate   	 Update] " << wilo_simulation_pump.flow_rate << " L/min" << std::endl;       
    }

  private: 
    pumpProto wilo_simulation_pump; 
    driver_registery driver_info;
};

#endif 
