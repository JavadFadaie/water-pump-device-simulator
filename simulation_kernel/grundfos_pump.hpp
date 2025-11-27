#ifndef GRUNDFOS_PUMP_HPP
#define GRUNDFOS_PUMP_HPP

#include "driver_base.hpp"
#include "driver_registery.hpp"
#include "driverId.hpp"

class grundfos_pump: public driver_base {

  public: 
    grundfos_pump()
    :driver_base(grundfos_simulation_pump)
    {}
    
    void driver_init_info()
    {
      driver_info = driver_registery(static_cast<int>(driverId::PUMP_GRUNDFOS), "Grundfos Pump Driver", "Grundfos");
    }

    void set_devices() override
    {
          device_list = 
          { 
						{ .device_name = "Grundfos CRE 20", .max_flow_rate=20.0, .max_pressure = 2, .power = 500}, 
					  { .device_name = "Grundfos CRE 30", .max_flow_rate=30.0, .max_pressure = 3, .power = 600}
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
        std::cout << "[Driver ID: " << static_cast<int>(driverId::PUMP_GRUNDFOS) << "] ";
        std::cout << "[Flow Rate Update] " << grundfos_simulation_pump.flow_rate << " L/min" << std::endl;
    }

  private: 
    pumpProto grundfos_simulation_pump; 
    driver_registery driver_info;
};

#endif 
