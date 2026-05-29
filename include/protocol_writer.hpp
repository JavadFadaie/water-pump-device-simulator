#ifndef PROTOCOL_WRITER_HPP
#define PROTOCOL_WRITER_HPP

#include <cstdint>

template<typename RegType, typename AddrType>
class ProtocolWriter
{
public:
    virtual ~ProtocolWriter() = default;
    virtual void addRegisterBlock(RegType type, AddrType startAddr, AccessType access, uint16_t count) = 0;
    virtual void setRegisterValue(RegType type, AddrType addr, uint16_t value) = 0;
    virtual void setRegisterValueFloat(RegType type, AddrType addr, float value) = 0;
    virtual uint16_t getRegisterValue(RegType type, AddrType addr) const = 0;
};

#endif
