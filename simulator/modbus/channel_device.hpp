#ifndef CHANNEL_DEVICE_HPP
#define CHANNEL_DEVICE_HPP

#include "modbus_types.hpp"
#include "protocol_writer.hpp"
#include <map>
#include <string>
#include <cstring>
#include <stdexcept>

struct extended_channel_info
{
    RegisterType regType;
    uint16_t address;
    uint8_t unit_id;
    float value;
    std::string name;
};

template<typename RegType, typename AddrType>
class channel_device
{
public:
    explicit channel_device(ProtocolWriter<RegType, AddrType>& writer)
        : writer_(writer)
    {}

    void addChannel(const std::string& name, RegType type, AddrType addr, uint8_t unitId = 1)
    {
        extended_channel_info info;
        info.regType = type;
        info.address = addr;
        info.unit_id = unitId;
        info.value = 0.0f;
        info.name = name;
        channels_[name] = info;
    }

    void setRegisterValueFloat(const std::string& channelName, float value, float scale = 1.0f)
    {
        auto it = channels_.find(channelName);
        if (it == channels_.end()) return;

        float scaled = value * scale;
        it->second.value = scaled;
        writer_.setRegisterValueFloat(it->second.regType, it->second.address, scaled);
    }

    void setRegisterValue(const std::string& channelName, uint16_t value)
    {
        auto it = channels_.find(channelName);
        if (it == channels_.end()) return;

        it->second.value = static_cast<float>(value);
        writer_.setRegisterValue(it->second.regType, it->second.address, value);
    }

    float getChannelValue(const std::string& channelName) const
    {
        auto it = channels_.find(channelName);
        if (it != channels_.end())
        {
            return it->second.value;
        }
        return 0.0f;
    }

    const std::map<std::string, extended_channel_info>& getChannels() const
    {
        return channels_;
    }

private:
    ProtocolWriter<RegType, AddrType>& writer_;
    std::map<std::string, extended_channel_info> channels_;
};

#endif
