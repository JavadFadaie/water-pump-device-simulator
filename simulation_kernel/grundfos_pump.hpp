#ifndef GRUNDFOS_PUMP_HPP
#define GRUNDFOS_PUMP_HPP

#include "driver_base.hpp"

class grundfos_pump: public driver_base {

  public:
    static constexpr int DRIVER_ID = 1;

    grundfos_pump()
    :driver_base(grundfos_simulation_pump)
    {}

    void set_devices() override
    {
          device_list =
          {
							{ .device_name = "Grundfos CRE 20", .max_flow_rate=20.0, .max_pressure = 2, .power = 500},
						  { .device_name = "Grundfos CRE 30", .max_flow_rate=30.0, .max_pressure = 3, .power = 600}
						};
    }

    void update_driver_value() override
    {
        std::cout << "[Driver ID: " << DRIVER_ID << "] ";
        std::cout << "[Flow Rate Update] " << grundfos_simulation_pump.flow_rate << " L/min" << std::endl;
    }

  private:
    pumpProto grundfos_simulation_pump;
};

#endif
