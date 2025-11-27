#include <iostream>
#include "pump_factory.hpp"
#include <string>
#include <thread>
#include <chrono>

int main()
{
    pump_factory factory;
    factory.choose_pump_driver(static_cast<int>(driverId::PUMP_WILO));
    
	return 0;
}
