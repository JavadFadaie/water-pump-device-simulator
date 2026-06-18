#ifndef SIM_VAR_HPP
#define SIM_VAR_HPP

#include <iostream>
#include <string>

struct pumpProto
{
	//Pump state
 	bool pump_on = false;
 	float pump_power = 0.0f;

 	//Tank/water
	bool tank_available = true;
 	float water_level = 0.0f;
 	float max_water_level = 0.0f;

 	//Flow
 	float flow_rate = 0.0f;
 	float pressure = 0.0f;

	//Time
	float pump_run_time = 0.0f;

	//Control
	bool auto_control = false;
	float threshold_low = 0.0f;
	float threshold_high = 0.0f;
};

#endif
