#ifndef DRIVER_BASE_HPP
#define DRIVER_BASE_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <memory>
#include <atomic>
#include <mutex>
#include "simulation_variable.hpp"
#include "modbus_core.hpp"
#include "channel_device.hpp"

class driver_base{

  public:
	driver_base(pumpProto & sim_value)
	: selected_device(std::make_unique<device_properties>())
	, pump(sim_value)
	, running{false}
	, simulation_duration{0}
	{}

	virtual ~driver_base()
	{
		stop_simulation();
	}

	virtual void set_devices() = 0;

	void setModbusCore(std::shared_ptr<ModbusCore> core)
	{
		modbus_core_ = core;
		channel_dev_ = std::make_unique<channel_device<RegisterType, uint16_t>>(*core);
		initChannels();
	}

	virtual void device_selection(std::string & device_name)
	{
		auto it = std::find_if( device_list.begin(), device_list.end(), [&device_name](const device_properties & device)
					{
						return device.device_name == device_name;
					});

    	if ( it != device_list.end() )
		{
        	std::cout << "Device found: " << it->device_name << std::endl;
			*selected_device = *it;
    	}
		else
		{
        	std::cout << "Device not found" << std::endl;
			selected_device = nullptr;
    	}
	}

	bool select_device_by_index(int index)
	{
		if (index < 0 || index >= static_cast<int>(device_list.size()))
		{
			return false;
		}
		*selected_device = device_list[index];
		return true;
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
		std::lock_guard<std::mutex> lock(pump_mutex);
		pump.pump_on = true;
		if(pump.pump_on && selected_device)
		{
			pump.flow_rate 	= selected_device->max_flow_rate + static_cast<float>(std::rand() % 500) / 100.0f;
			pump.pressure 	= selected_device->max_pressure  + static_cast<float>(std::rand() % 100) / 50.0f;
			pump.pump_power = selected_device->power		 + static_cast<float>(std::rand() % 40);
			pump.pump_run_time += 1.0f;
			pump.water_level += pump.flow_rate / 60.0f;
		}
	}

	void simulate_driver_values()
	{
		set_simulation_status(true);
		while (running)
		{
        	simulate_values();
        	update_driver_value();
        	std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void start_simulation()
	{
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
		auto start_time = std::chrono::steady_clock::now();
		auto duration = std::chrono::seconds(simulation_duration);

		start_simulation();

		while(std::chrono::steady_clock::now() - start_time < duration)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		stop_simulation();
    }

	virtual void set_simulation_duration(int duration_seconds)
	{
		simulation_duration = duration_seconds;
	}

	virtual void driver_simulation()
	{
		generate_simulation();
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

	const pumpProto& get_pump_data() const
	{
		return pump;
	}

	pumpProto& get_pump_data()
	{
		return pump;
	}

	const std::vector<device_properties>& get_device_list() const
	{
		return device_list;
	}

	bool is_running() const
	{
		return running.load();
	}

	std::mutex& get_mutex()
	{
		return pump_mutex;
	}

  protected:
	std::vector<device_properties> device_list;
	pumpProto & pump;
	std::atomic<bool> running;
	std::unique_ptr<device_properties>  selected_device;
	std::thread simulation_thread;
	int simulation_duration;
	std::mutex pump_mutex;

	std::shared_ptr<ModbusCore> modbus_core_;
	std::unique_ptr<channel_device<RegisterType, uint16_t>> channel_dev_;

	virtual void initChannels()
	{
		if (!channel_dev_) return;
		channel_dev_->addChannel("CH_FLOW_RATE",   RegisterType::InputRegister, REG_FLOW_RATE);
		channel_dev_->addChannel("CH_PRESSURE",    RegisterType::InputRegister, REG_PRESSURE);
		channel_dev_->addChannel("CH_PUMP_POWER",  RegisterType::InputRegister, REG_PUMP_POWER);
		channel_dev_->addChannel("CH_WATER_LEVEL", RegisterType::InputRegister, REG_WATER_LEVEL);
		channel_dev_->addChannel("CH_RUN_TIME",    RegisterType::InputRegister, REG_RUN_TIME);
		channel_dev_->addChannel("CH_PUMP_ON",     RegisterType::InputRegister, REG_PUMP_ON);
	}

	void writeSimulationToRegisters()
	{
		if (!channel_dev_) return;

		channel_dev_->setRegisterValueFloat("CH_FLOW_RATE",   pump.flow_rate,     1.0f);
		channel_dev_->setRegisterValueFloat("CH_PRESSURE",    pump.pressure,      1.0f);
		channel_dev_->setRegisterValueFloat("CH_PUMP_POWER",  pump.pump_power,    1.0f);
		channel_dev_->setRegisterValueFloat("CH_WATER_LEVEL", pump.water_level,   1.0f);
		channel_dev_->setRegisterValueFloat("CH_RUN_TIME",    pump.pump_run_time, 1.0f);
		channel_dev_->setRegisterValue("CH_PUMP_ON", pump.pump_on ? 1 : 0);

		if (modbus_core_)
		{
			modbus_core_->setRegisterValueFloat(RegisterType::HoldingRegister, REG_FLOW_RATE,   pump.flow_rate);
			modbus_core_->setRegisterValueFloat(RegisterType::HoldingRegister, REG_PRESSURE,    pump.pressure);
			modbus_core_->setRegisterValueFloat(RegisterType::HoldingRegister, REG_PUMP_POWER,  pump.pump_power);
			modbus_core_->setRegisterValueFloat(RegisterType::HoldingRegister, REG_WATER_LEVEL, pump.water_level);
			modbus_core_->setRegisterValueFloat(RegisterType::HoldingRegister, REG_RUN_TIME,    pump.pump_run_time);
			modbus_core_->setRegisterValue(RegisterType::HoldingRegister, REG_PUMP_ON, pump.pump_on ? 1 : 0);
		}
	}
};


#endif
