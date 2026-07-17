#AGENTS.md — modbus/

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder is the only place in the project that calls `<modbus/modbus.h>` directly. All other code reads/writes pump data through `ModbusReader` and `ModbusWriter`, which are transport-agnostic.

---

## Files

| File | Role |
|------|------|
| `modbus_transport.hpp/cpp` | `ModbusTransport` abstract base; `TcpTransport` and `RtuTransport` concrete implementations |
| `modbus_reader.hpp/cpp`    | FC4 `read_input_registers` — decodes sensor floats; works with any transport |
| `modbus_writer.hpp/cpp`    | FC16 `write_multiple_registers` — encodes setpoints; works with any transport |

---

## ModbusTransport

Owns the `modbus_t*` context. `ModbusReader` and `ModbusWriter` hold a `ModbusTransport*` injected by `main.cpp` — they never call `modbus_new_tcp()` or `modbus_new_rtu()` directly.

```cpp
class ModbusTransport {
public:
    virtual ~ModbusTransport() = default;
    virtual bool connect()    = 0;
    virtual void disconnect() = 0;
    modbus_t* ctx() const { return ctx_; }
protected:
    modbus_t* ctx_ = nullptr;
};

class TcpTransport : public ModbusTransport {
public:
    TcpTransport(const std::string& host, int port, int slave_id);
    bool connect() override;    // modbus_new_tcp + modbus_set_slave + modbus_connect
    void disconnect() override; // modbus_close + modbus_free
};

class RtuTransport : public ModbusTransport {
public:
    RtuTransport(const std::string& device, int baud,
                 char parity, int data_bits, int stop_bits, int slave_id);
    bool connect() override;    // modbus_new_rtu + modbus_set_slave + modbus_connect
    void disconnect() override; // modbus_close + modbus_free
};
```

`main.cpp` constructs the right transport from `config.json` and injects it:

```cpp
std::unique_ptr<ModbusTransport> transport;
if (cfg.transport == "tcp")
    transport = std::make_unique<TcpTransport>(cfg.tcp.host, cfg.tcp.port, cfg.tcp.slave_id);
else
    transport = std::make_unique<RtuTransport>(cfg.rtu.device, cfg.rtu.baud,
                                               cfg.rtu.parity[0], cfg.rtu.data_bits,
                                               cfg.rtu.stop_bits, cfg.rtu.slave_id);
transport->connect();

ModbusReader reader(*transport);
ModbusWriter writer(*transport);
```

---

## ModbusReader

Takes a `ModbusTransport&`. Uses `transport.ctx()` for all libmodbus calls.

```cpp
class ModbusReader {
public:
    explicit ModbusReader(ModbusTransport& transport);

    // Reads 11 input registers starting at base_addr, returns populated snapshot.
    // base_addr = 100 (Grundfos) or 200 (Wilo)
    PumpSnapshot readSnapshot(uint16_t base_addr);

private:
    float    readFloat(uint16_t addr);   // reads 2 registers → memcpy → float
    uint16_t readUint16(uint16_t addr);  // reads 1 register
    ModbusTransport& transport_;
};
```

### Float Decoding

The simulator stores each float as two consecutive uint16 big-endian words (IEEE 754):

```cpp
uint16_t regs[2];
modbus_read_input_registers(transport_.ctx(), addr, 2, regs);

float value;
uint32_t raw = ((uint32_t)regs[0] << 16) | regs[1];
memcpy(&value, &raw, sizeof(float));
```

This matches `ModbusCore::setRegisterValueFloat()` in the simulator.

---

## ModbusWriter

Takes the same `ModbusTransport&`. All function codes work identically over TCP and RTU.

```cpp
class ModbusWriter {
public:
    explicit ModbusWriter(ModbusTransport& transport);

    bool writeFloat(uint16_t addr, float value);     // FC16, 2 registers
    bool writeUint16(uint16_t addr, uint16_t value); // FC6,  1 register
};
```

Float encoding (reverse of decoding):

```cpp
uint32_t raw;
memcpy(&raw, &value, sizeof(float));
uint16_t regs[2] = { (uint16_t)(raw >> 16), (uint16_t)(raw & 0xFFFF) };
modbus_write_registers(transport_.ctx(), addr, 2, regs);
```

---

## Error Handling

- On `modbus_read_input_registers` returning `-1`: log `modbus_strerror(errno)`, call `modbus_flush()`, attempt one reconnect via `transport_.connect()`
- On reconnect failure: return a `PumpSnapshot` with `valid = false` (caller checks before using values)
- RTU-specific: `errno == ETIMEDOUT` on serial is normal (no device on bus) — log as warning, do not treat as fatal
- Writer: on failure, log the error; caller decides whether to retry

---

## Transport Comparison

| | TcpTransport | RtuTransport |
|---|---|---|
| libmodbus init | `modbus_new_tcp(host, port)` | `modbus_new_rtu(device, baud, parity, data_bits, stop_bits)` |
| Connect | `modbus_connect()` | `modbus_connect()` (opens serial fd) |
| Slave set | `modbus_set_slave()` | `modbus_set_slave()` |
| Reconnect strategy | re-create TCP socket | re-open serial port |
| Timeout handling | `ETIMEDOUT` → retry | `ETIMEDOUT` → log warning only |
| Config source | `config.json` → `tcp` object | `config.json` → `rtu` object |

---

## Planned Control Register Map (simulator extension required)

When holding registers are added to the simulator, this will be the target map (same addresses for both transports):

| Address | Count | Type   | Channel            | Unit  |
|---------|-------|--------|--------------------|-------|
| 500     | 1     | UInt16 | Pump On Command    | 0/1   |
| 502–503 | 2     | Float  | Flow Rate Setpoint | L/min |
| 504–505 | 2     | Float  | Pressure Setpoint  | bar   |

Base address 500 avoids conflict with Grundfos (100) and Wilo (200). Wilo's next available is 300, so 500 gives ample headroom.
