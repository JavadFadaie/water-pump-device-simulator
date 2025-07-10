#ifndef SIM_VAR_HPP
#define SIM_VAR_HPP

#include <iostream>
#include <string>

struct pumpProto
{
	//Pump state
 	bool pump_on;
 	float pump_power;
 	
 	//Tank/water
	bool tank_available = true;  
 	float water_level;
 	float max_water_level;
 	
 	//Flow
 	float flow_rate;
 	float pressure;
 	
	//Time
	float pump_run_time;
	
	//Control
	bool auto_control;
	float threshold_low;
	float threshold_high;
};


struct device_properties
{
	std::string device_name;
	float max_flow_rate;
	float max_pressure;
	float power;
};

#endif
