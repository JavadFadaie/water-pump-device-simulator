# AGENTS.md — WPCM_core (Water Pump Control & Monitor)

## Project Overview

C++17 application that connects to a running **simulator** instance and provides:

- **Monitoring** — periodic polling of Modbus input registers (sensor values), in-memory history, CSV logging, threshold alarms
- **Control** — sending control commands to the simulator via its REST API (start/stop pump) and writing setpoints to Modbus holding registers (flow rate target, on/off command)

This project is a **Modbus client** only — it never acts as a server. It supports two transports, selected at startup via `config.json`:

| Transport | When to use |
|-----------|-------------|
| **Modbus TCP** | Simulator running on same host or LAN (`127.0.0.1:1502`) |
| **Modbus RTU (RS-485)** | Physical pump or simulator connected over serial (`/dev/ttyUSB0`, 9600 8N1) |

Both transports use identical register maps and the same `ModbusReader`/`ModbusWriter` interface — only the underlying `modbus_t*` context differs.

---

## Simulator Interface (external dependency)

The **simulator** (`../simulator`) must be running before this system starts.

| Interface | Connection | Purpose |
|-----------|------------|---------|
| Modbus TCP | `127.0.0.1:1502` | Read sensor registers (FC4), write control registers (FC16) |
| Modbus RTU | `/dev/ttyUSB0`, 9600 8N1, slave 1 | Same register map over RS-485 serial |
| REST API   | `127.0.0.1:8085` | Start/stop simulation, query status (TCP mode only) |

### Simulator Modbus Register Map (read-only Input Registers, FC4)

#### Wilo driver (slave ID 1, base address 200)

| Address | Count | Type   | Channel       | Unit  |
|---------|-------|--------|---------------|-------|
| 200–201 | 2     | Float  | Flow Rate     | L/min |
| 202–203 | 2     | Float  | Pressure      | bar   |
| 204–205 | 2     | Float  | Pump Power    | W     |
| 206–207 | 2     | Float  | Water Level   | L     |
| 208–209 | 2     | Float  | Run Time      | s     |
| 210     | 1     | UInt16 | Pump On       | 0/1   |

#### Grundfos driver (slave ID 1, base address 100)

| Address | Count | Type   | Channel       | Unit  |
|---------|-------|--------|---------------|-------|
| 100–101 | 2     | Float  | Flow Rate     | L/min |
| 102–103 | 2     | Float  | Pressure      | bar   |
| 104–105 | 2     | Float  | Pump Power    | W     |
| 106–107 | 2     | Float  | Water Level   | L     |
| 108–109 | 2     | Float  | Run Time      | s     |
| 110     | 1     | UInt16 | Pump On       | 0/1   |

Float encoding: two consecutive uint16 words, big-endian IEEE 754. Use `struct.unpack('>f', ...)` or the equivalent C cast via `memcpy`.

### Simulator REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET  | `/api/drivers` | List registered drivers |
| GET  | `/api/drivers/{id}/models` | List models for a driver |
| POST | `/api/simulation/start` | Start simulation (body: `{"driver_id":1,"model_id":0}`) |
| POST | `/api/simulation/stop`  | Stop running simulation |
| GET  | `/api/simulation/status` | Live sensor values |

---

## Architecture

```
main.cpp
  |
  +-- ModbusTransport (factory)
  |     creates either ModbusTcpContext or ModbusRtuContext
  |     both expose the same modbus_t* handle
  |
  +-- MonitorLoop (thread, every poll_interval_ms)
  |     +-- ModbusReader        → FC4 read_input_registers(addr, count) via transport
  |     +-- PumpDataStore       → ring buffer of PumpSnapshot structs
  |     +-- AlarmManager        → checks thresholds, emits AlarmEvent
  |     +-- DataLogger          → appends CSV rows to log file
  |
  +-- ControlClient
  |     +-- ModbusWriter        → FC16 write_registers (setpoints/commands) via transport
  |     +-- HttpClient          → REST calls to simulator (start/stop, TCP mode only)
  |
  +-- (optional) WebServer      → exposes /api/status and /api/control for UI or SCADA
```

---

## Sub-Agent Index

| Sub-Agent | Covers |
|-----------|--------|
| [modbus/AGENTS.md](modbus/AGENTS.md)   | Transport abstraction, TCP/RTU contexts, ModbusReader (FC4), ModbusWriter (FC16), float decode |
| [monitor/AGENTS.md](monitor/AGENTS.md) | Poll loop, PumpDataStore ring buffer, AlarmManager, DataLogger |
| [control/AGENTS.md](control/AGENTS.md) | ControlClient, setpoint management, REST control, Modbus write path |
| [include/AGENTS.md](include/AGENTS.md) | Shared types: PumpSnapshot, AlarmEvent, config structs (TCP + RTU variants) |
| [docs/AGENTS.md](docs/AGENTS.md)       | Documentation index |

---

## Directory Structure

```
WPCM_core/
├── main.cpp                        # Entry point: wires MonitorLoop + ControlClient
├── CMakeLists.txt                  # libmodbus (system), cpp-httplib (fetched), nlohmann/json (fetched)
├── AGENTS.md                       # This file — master orchestrator
├── include/
│   ├── AGENTS.md
│   ├── pump_snapshot.hpp           # PumpSnapshot struct (timestamped sensor reading)
│   ├── alarm_event.hpp             # AlarmEvent struct + AlarmLevel enum
│   ├── pump_config.hpp             # Connection config: transport type, TCP/RTU params, poll interval
│   └── driver_map.hpp              # Maps driver_id → base_address (Grundfos=100, Wilo=200)
├── modbus/
│   ├── AGENTS.md
│   ├── modbus_transport.hpp/cpp    # ModbusTransport abstract base; TcpTransport / RtuTransport
│   ├── modbus_reader.hpp/cpp       # FC4 read_input_registers; decodes float pairs; transport-agnostic
│   └── modbus_writer.hpp/cpp       # FC16 write_registers; encodes float/uint16; transport-agnostic
├── monitor/
│   ├── AGENTS.md
│   ├── monitor_loop.hpp/cpp        # Polling thread, calls reader every poll_interval_ms
│   ├── pump_data_store.hpp/cpp     # Thread-safe ring buffer (N snapshots)
│   ├── alarm_manager.hpp/cpp       # Threshold config → AlarmEvent on breach
│   └── data_logger.hpp/cpp         # Appends PumpSnapshot to CSV (logs/pump_YYYYMMDD.csv)
├── control/
│   ├── AGENTS.md
│   ├── control_client.hpp/cpp      # Public API: start(), stop(), setFlowSetpoint(float)
│   ├── http_client.hpp/cpp         # Thin wrapper around httplib for simulator REST calls
│   └── setpoint_manager.hpp/cpp    # Holds current setpoints, detects changes, triggers writes
└── docs/
    └── AGENTS.md
```

---

## Key Design Patterns

- **Transport abstraction** (`ModbusTransport`): `ModbusReader` and `ModbusWriter` hold a `ModbusTransport*` and never call `modbus_new_tcp()` or `modbus_new_rtu()` directly. `main.cpp` creates the right concrete transport from `config.json` and injects it. See [modbus/AGENTS.md](modbus/AGENTS.md).
- **Shared ModbusCore — NOT used here**: The simulator uses `ModbusCore` internally. This project talks to it purely over the wire as a real PLC/SCADA would — no shared memory, no shared headers.
- **Float decoding**: Every float spans two consecutive uint16 registers. `ModbusReader::readFloat(addr)` calls `read_input_registers(addr, 2)` and reassembles via `memcpy` into a `float`. See [modbus/AGENTS.md](modbus/AGENTS.md).
- **PumpSnapshot**: All data at a single poll instant is packed into one `PumpSnapshot` (timestamp + 6 values). This is the unit passed between monitor → store → logger → alarm.
- **Control via two paths**:
  1. **REST** (`HttpClient`) — for lifecycle (start/stop simulation, select driver/model); TCP only
  2. **Modbus FC16** (`ModbusWriter`) — for runtime setpoints; works over both TCP and RTU

---

## Build and Run

```bash
# Simulator must be running first
cd ../simulator
./build/pump_simulation &

# Then build and run this project
cd ../WPCM_core
mkdir -p build && cd build
cmake .. && cmake --build .
./pump_monitor
```

**Dependencies:** C++17, libmodbus (system), cpp-httplib (fetched), nlohmann/json (fetched), pthreads

---

## Configuration

Runtime config is loaded from `config.json` at startup. The `transport` field selects TCP or RTU; only the relevant sub-object is read.

**TCP example:**
```json
{
  "transport": "tcp",
  "tcp": {
    "host": "127.0.0.1",
    "port": 1502,
    "slave_id": 1
  },
  "rest_port": 8085,
  "driver_id": 1,
  "model_id": 0,
  "poll_interval_ms": 1000,
  "log_dir": "logs/",
  "alarms": {
    "flow_rate_max": 60.0,
    "pressure_max": 4.0,
    "pump_power_max": 600.0,
    "water_level_max": 100.0
  }
}
```

**RTU example:**
```json
{
  "transport": "rtu",
  "rtu": {
    "device": "/dev/ttyUSB0",
    "baud": 9600,
    "parity": "N",
    "data_bits": 8,
    "stop_bits": 1,
    "slave_id": 1
  },
  "driver_id": 1,
  "model_id": 0,
  "poll_interval_ms": 1000,
  "log_dir": "logs/",
  "alarms": {
    "flow_rate_max": 60.0,
    "pressure_max": 4.0,
    "pump_power_max": 600.0,
    "water_level_max": 100.0
  }
}
```

> `rest_port` is ignored in RTU mode — lifecycle control (start/stop) is not available over serial.

---

## Current Limitations / Future Work

- Simulator input registers are read-only (FC4); control via Modbus FC16 requires adding holding registers to the simulator for setpoints
- No persistent alarm history yet (AlarmManager emits events in-memory only)
- Single pump / single driver at a time — multi-pump support requires multiple `MonitorLoop` instances with different base addresses
- No web UI in v1 — monitoring output is console + CSV log only
