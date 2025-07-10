#include <iostream>
#include "simulation_variable.hpp"
#include "grundfos_pump.hpp"
#include <string>



int main(){


	
	grundfos_pump  pump;
    pump.set_devices(); // Populate device_list
	
    pump.display_device(); // Display available devices
    
    std::string device_name = "Grundfos CRE 20"; // Valid device name
    pump.device_selection(device_name);
    pump.simualte_driver_values(); // Corrected function name
	
	return 0;
}
