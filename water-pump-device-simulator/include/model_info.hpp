#ifndef MODEL_INFO_HPP
#define MODEL_INFO_HPP

#include <string>
#include <vector>

enum class CommProtocol
{
    PROTOCOL_NONE = 0,
    PROTOCOL_MODBUS_TCP,
    PROTOCOL_MODBUS_RTU,
    PROTOCOL_OPCUA,
    PROTOCOL_MQTT
};

enum class DeviceType
{
    Pump = 0,
    BoosterSystem,
    CirculationPump
};

enum class DriverId : int
{
    GRUNDFOS_PUMP = 1,
    WILO_PUMP     = 2
};

struct ModelInfo
{
    int          deviceId;
    std::string  Name;
    float        max_flow_rate;
    float        max_pressure;
    float        power;
    DeviceType   deviceType     = DeviceType::Pump;
    bool         SupportEth     = false;
    bool         SupportRS485   = false;
    CommProtocol EthProtocol    = CommProtocol::PROTOCOL_NONE;
    CommProtocol SerialProtocol = CommProtocol::PROTOCOL_NONE;
};

#endif
