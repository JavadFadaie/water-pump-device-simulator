#ifndef WILO_PUMP_HPP
#define WILO_PUMP_HPP

#include "driver_base.hpp"

class wilo_pump: public driver_base{

    public: 

    wilo_pump()
    :driver_base()
    {}
    
    void set_devices() override
    {
        device_list = { 
						{ .device_name = "Stratos MAXO 50", .max_flow_rate=50.0, .max_pressure = 2, .power = 500}, 
					   	{ .device_name = "Stratos MAXO 60", .max_flow_rate=60.0, .max_pressure = 3, .power = 600}
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
