#ifndef WILO_PUMP_HPP
#define WILO_PUMP_HPP

#include "driver_base.hpp"

class wilo_pump: public driver_base
{
  public:
    static constexpr int DRIVER_ID = 2;

    wilo_pump()
    :driver_base(wilo_simulation_pump)
    {}

    void set_devices() override
    {
        device_list =
        {
	          { .device_name = "Stratos MAXO 50", .max_flow_rate=50.0, .max_pressure = 2, .power = 500},
		        { .device_name = "Stratos MAXO 60", .max_flow_rate=60.0, .max_pressure = 3, .power = 600}
        };
    }

    void update_driver_value() override
    {
       std::cout << "[Driver ID: " << DRIVER_ID << "] ";
       std::cout << "[Flow Rate   	 Update] " << wilo_simulation_pump.flow_rate << " L/min" << std::endl;
    }

  private:
    pumpProto wilo_simulation_pump;
};

#endif
