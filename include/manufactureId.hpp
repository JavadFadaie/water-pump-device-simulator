#ifndef MANUFACTURERID_HPP
#define MANUFACTURERID_HPP

#include <string>

struct manufactorelist_t
{
    int manufactureid;
    std::string manufacture_name;
};

enum class ManufacturerId
{
    MID_GRUNDFOS   = 1,
    MID_WILO       = 2,
};

const manufactorelist_t manufacture_list[] = 
{
    { static_cast<int>(ManufacturerId::MID_GRUNDFOS), "Grundfos" },
    { static_cast<int>(ManufacturerId::MID_WILO), "Wilo" },
};

#endif 