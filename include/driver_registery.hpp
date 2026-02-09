#ifndef DRIVER_REGISTERY_HPP
#define DRIVER_REGISTERY_HPP

#include <string>

class driver_registery
{
  public : 
  driver_registery
        ( int _driverId
        , std::string _driver_name
        , int _manufacturer_id)
        : driverId(_driverId)
        , driver_name(_driver_name)
        , manufacturer_id(_manufacturer_id)
        {}
   
   driver_registery() = default;     

  protected : 
    int driverId;
    std::string driver_name;
    int manufacturer_id; 
};

#endif
