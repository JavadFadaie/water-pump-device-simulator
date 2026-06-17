# AGENTS.md — Modbus Protocol Stack

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains the complete Modbus abstraction layer. It is the only part of the codebase that calls the libmodbus C API directly (`modbus_server.cpp`). Everything else interacts through the interfaces defined here.

---

## Files

| File | Role |
|------|------|
| `modbus_types.hpp` | Foundational types: `RegisterType`, `AccessType`, `ModbusRegister` struct |
| `protocol_writer.hpp` | Abstract template interface `ProtocolWriter<RegType, AddrType>` — decouples simulation from protocol |
| `modbus_core.hpp` | Thread-safe in-memory register store; implements `ProtocolWriter`; encodes floats as two IEEE 754 16-bit words |
| `modbus_server.hpp` | `ModbusServer` (abstract), `ModbusTcp` (TCP transport), `ModbusRtu` (RTU/serial transport) declarations |
| `modbus_server.cpp` | Only file calling `<modbus/modbus.h>`; `select()`-based server loop for both TCP and RTU; copies `ModbusCore` state via `syncToMapping()` before each reply |
| `channel_device.hpp` | Maps human-readable channel names (`"CH_FLOW_RATE"`) to register addresses; sits between pump drivers and `ProtocolWriter` |

---

## Data Flow

### TCP (default)
```
Driver constructor
  → ModbusTcp::ModbusTcp(unit_id)        creates ModbusCore
  → ModbusCore::addRegisterBlock(...)    reserves address range
  → channel_device::addChannel(name, addr, type)

Simulation tick (every 1 s)
  → driver::writeSimulationToRegisters()
      → channel_device::setRegisterValueFloat("CH_FLOW_RATE", value)
          → ModbusCore::setRegisterValueFloat(type, addr, value)
              → stores two uint16_t words in internal map

Modbus TCP client query (any time)
  → ModbusTcp::serverLoop()  [select() on TCP socket, port 1502]
      → ModbusCore::syncToMapping(mapping)
      → modbus_reply(ctx, query, mapping)
```

### RTU (RS-485, activated via API)
```
POST /api/simulation/start  {"interface": "rs485", "device": "/dev/ttyUSB0", "baud": 9600}
  → driver_base::configureRtu(device, baud)
      → shares existing ModbusCore with new ModbusRtu instance
      → previous ModbusTcp is destroyed

Modbus RTU master query (any time)
  → ModbusRtu::serverLoop()  [select() on serial fd]
      → modbus_receive(ctx, query)
      → ModbusCore::syncToMapping(mapping)
      → modbus_reply(ctx, query, mapping)
      → on error (rc == -1, errno != ETIMEDOUT): log + modbus_flush()
```

The `ModbusCore` register state is shared between TCP and RTU — switching transport does not reset any register values.

---

## Transports

| | ModbusTcp | ModbusRtu |
|---|---|---|
| Default port / device | `0.0.0.0:1502` | `/dev/ttyUSB0` |
| Default baud | — | 9600 |
| Serial framing | — | 8N1 |
| Triggered by | driver constructor | `configureRtu()` / `"interface":"rs485"` in start API |
| Models that support it | all | models with `SupportRS485 = true` |

---

## Register Maps

### Grundfos (base address 100)

| Address | Size | Type   | Channel            |
|---------|------|--------|--------------------|
| 100–101 | 2    | Float  | Flow Rate (L/min)  |
| 102–103 | 2    | Float  | Pressure (bar)     |
| 104–105 | 2    | Float  | Pump Power (W)     |
| 106–107 | 2    | Float  | Water Level (L)    |
| 108–109 | 2    | Float  | Run Time (s)       |
| 110     | 1    | UInt16 | Pump On (0/1)      |

### Wilo (base address 200)

| Address | Size | Type   | Channel            |
|---------|------|--------|--------------------|
| 200–201 | 2    | Float  | Flow Rate (L/min)  |
| 202–203 | 2    | Float  | Pressure (bar)     |
| 204–205 | 2    | Float  | Pump Power (W)     |
| 206–207 | 2    | Float  | Water Level (L)    |
| 208–209 | 2    | Float  | Run Time (s)       |
| 210     | 1    | UInt16 | Pump On (0/1)      |

Registers are written as both Input Registers and Holding Registers. New drivers must use a unique base address (next available: 300).

---

## Adding a New Protocol (OPC-UA, MQTT, etc.)

1. Create a new class implementing `ProtocolWriter<YourRegType, YourAddrType>`
2. Implement `addRegisterBlock()`, `setRegisterValue()`, `setRegisterValueFloat()`, `getRegisterValue()`
3. In the driver constructor, replace `ModbusTcp` with your new server — `driver_base::mModbus` accepts any `ModbusServer`-derived type (or adapt the base class if the interface differs)
4. No changes needed to `channel_device`, simulation logic, or web server

---

## Known Limitations

- Only one Modbus server can bind port 1502 at a time; stopping a driver releases the port
