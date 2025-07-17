#include <iostream>
#include "simulation_variable.hpp"
#include "grundfos_pump.hpp"
#include <string>
#include <thread>
#include <chrono>


int main(){


	
	grundfos_pump  pump;
    pump.set_devices(); // Populate device_list
	pump.display_device(); // Display available devices
    std::string device_name = "Grundfos CRE 20"; // Valid device name
    pump.device_selection(device_name);
    // Start simulation in a separate thread

    pump.driver_simulation(60);

	
	return 0;
}
