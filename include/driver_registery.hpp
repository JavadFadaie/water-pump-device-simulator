#ifndef DRIVER_REGISTERY_HPP
#define DRIVER_REGISTERY_HPP

#include <string>

class driver_registery{

    public : 

        driver_registery
            ( int _driverId
            , std::string _driver_name
            , std::string _manufacturer_name)
            : driverId(_driverId)
            , driver_name(_driver_name)
            , manufacturer_name(_manufacturer_name)
            {}


    protected : 
    
        int driverId;
        std::string driver_name;
        std::string manufacturer_name; 
};

#endif