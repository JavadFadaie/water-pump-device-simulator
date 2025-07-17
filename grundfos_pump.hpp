#ifndef GRUNDFOS_PUMP_HPP
#define GRUNDFOS_PUMP_HPP

#include "driver_base.hpp"

class grundfos_pump: public driver_base{

    public: 

    grundfos_pump()
    :driver_base()
    {}
    
    void set_devices() override
    {
        device_list = { 
						{ .device_name = "Grundfos CRE 20", .max_flow_rate=20.0, .max_pressure = 2, .power = 500}, 
					   	{ .device_name = "Grundfos CRE 30", .max_flow_rate=30.0, .max_pressure = 3, .power = 600}
					  };
    }

    void driver_simulation(int duration_seconds)
    {
        simulation_duration = duration_seconds;
        generate_simulation();
    }

    void update_driver_value(pumpProto & grundfos_value) override
    {
        std::cout << "[Flow Rate   	 Update] " << grundfos_value.flow_rate << " L/min" << std::endl;
           
    }



    private : 



};

#endif 
