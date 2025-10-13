#ifndef DRIVER_BASE_HPP
#define DRIVER_BASE_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <memory>
#include <atomic>
#include "simulation_variable.hpp" // Include the header for device_properties and pumpProto

class driver_base{

	public : 
	
	driver_base(pumpProto & sim_value)
	: selected_device(std::make_unique<device_properties>())
	, pump(sim_value)
	{}

	virtual ~driver_base() 
	{
		stop_simulation();
    }
	
	virtual void set_devices() = 0; // Make pure virtual

	virtual void device_selection(std::string & device_name)
	{
		auto it = std::find_if( device_list.begin(), device_list.end(), [&device_name](const device_properties & device) 
		{
            return device.device_name == device_name;
        }
    	);

    	if (it != device_list.end()) 
		{
        	std::cout << "Device found: " << it->device_name << std::endl;
			*selected_device = *it; // Copy the entire struct

    	}
		else
		{
        	std::cout << "Device not found" << std::endl;
			selected_device = nullptr; // Reset if not found
    	}
	}

	virtual void display_device()
	{
		for( auto i : device_list)
		{
			std::cout<< i.device_name << "  " << i.max_flow_rate << std::endl; 
		}
	}

	void simulate_values()
	{
		pump.pump_on = true;
		if(pump.pump_on && selected_device)
		{
			pump.flow_rate 	= selected_device->max_flow_rate + static_cast<float>(std::rand() % 500) / 100.0f; // 10.0 - 15.0 L/min
			pump.pressure 	= selected_device->max_pressure  + static_cast<float>(std::rand() % 100) / 50.0f;   // 1.0 - 3.0 bar
			pump.pump_power = selected_device->power		 + static_cast<float>(std::rand() % 40);          // 480 - 520 W
			pump.pump_run_time += 1.0f; 											  // simulate one more second
			pump.water_level += pump.flow_rate / 60.0f; 							  // increase tank level in each sec
		}	
	}

	void simulate_driver_values()
	{
		set_simulation_status(true);
		while (running) 
		{
        	simulate_values();  								// Simulate new flow_rate
        	update_driver_value();								// Print flow rate during simulation
			//print_update_driver_values();
        	std::this_thread::sleep_for(std::chrono::seconds(1));  // Wait 1 seconds
		}
	}

	void start_simulation() 
	{
		//The std::thread constructor creates a new thread and immediately starts 
		//executing driver_base::simulate_driver_values on the current object (this).
        if (!running) 
		{
            simulation_thread = std::thread(&driver_base::simulate_driver_values, this);
        }
    }

	void stop_simulation() 
	{
		set_simulation_status(false);
		if (simulation_thread.joinable()) 
		{
            simulation_thread.join();
        }
    }

	void set_simulation_status(bool status)
	{
		running = status;
	}

	virtual void generate_simulation()
    {
        start_simulation();
        auto start_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::seconds(simulation_duration);
     
        while(std::chrono::steady_clock::now() - start_time < duration)
        {
	        std::this_thread::sleep_for(std::chrono::seconds(1));
		    // Let the simulation thread handle the updates
        }
       
        stop_simulation();
    }

	virtual void update_driver_value() = 0;

	void print_update_driver_values() 
	{
		std::cout << "[Pump Run Time Update] " << pump.pump_run_time << " seconds" << std::endl;
    	std::cout << "[Flow Rate   	 Update] " << pump.flow_rate << " L/min" << std::endl;
    	std::cout << "[Pressure    	 Update] " << pump.pressure << " bar" << std::endl;
    	std::cout << "[Pump Power  	 Update] " << pump.pump_power << " W" << std::endl;
    	std::cout << "[Water Level 	 Update] " << pump.water_level << " Liter" << std::endl;
		std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< std::endl;
	}

	protected : 
	
	std::vector<device_properties> device_list;
	pumpProto & pump; 
	std::atomic<bool> running; // Control simulation loop
	std::unique_ptr<device_properties>  selected_device;
	std::thread simulation_thread; // Thread for simulation
	int simulation_duration;

};


#endif
