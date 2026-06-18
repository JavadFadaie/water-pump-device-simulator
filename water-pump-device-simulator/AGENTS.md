# AGENTS.md — SMCS_AWP Water Pump Device Simulator (Master)

## Project Overview

Industrial water pump device simulator written in C++17. Simulates pump sensor data and exposes it through an HTTP web dashboard (port 8085) and a Modbus TCP server (port 1502). Designed for testing SCADA/PLC integration without physical pump hardware.

---

## Sub-Agent Index

Each major component has its own AGENTS.md. Start here to navigate to the right area:

| Sub-Agent | Covers |
|-----------|--------|
| [modbus/AGENTS.md](modbus/AGENTS.md) | Modbus protocol stack, register maps, data flow, extending to new protocols |
| [simulation_kernel/AGENTS.md](simulation_kernel/AGENTS.md) | Pump driver architecture, self-registration, how to add a new driver |
| [include/AGENTS.md](include/AGENTS.md) | Core infrastructure: driver_base, registry, factory, web_server |
| [docs/AGENTS.md](docs/AGENTS.md) | Documentation index |

---

## Architecture

```
main.cpp
  |
  +-- web_server (port 8085, REST API + static UI)       → see include/AGENTS.md
        +-- driver_registry (singleton)                  → see include/AGENTS.md
              +-- driver_base (abstract, per-manufacturer)
                    +-- grundfos_pump / wilo_pump / ...  → see simulation_kernel/AGENTS.md
                          |
                          +-- ModbusTcp (port 1502)      → see modbus/AGENTS.md
                          |     +-- ModbusCore
                          |
                          +-- channel_device (named channels → registers)
```

Each driver owns its entire Modbus stack. `start_simulation()` starts both the simulation thread and the Modbus TCP server. `stop_simulation()` stops both, releasing port 1502 for the next driver.

---

## Key Design Patterns

- **Self-Registering Drivers**: Drivers register via `static volatile driver_registry::register_t` at static init — no central list to maintain. See [simulation_kernel/AGENTS.md](simulation_kernel/AGENTS.md).
- **Factory Pattern**: `driver_registry` creates instances by `DriverId` via a `new_instance()` function pointer.
- **Driver-Owned Modbus**: Each driver constructs its own `ModbusTcp`, `ModbusCore`, and `channel_device` — no shared Modbus state.
- **Protocol Abstraction**: `ProtocolWriter<RegType, AddrType>` decouples simulation from protocol. Future protocols (OPC-UA, MQTT) implement the same interface. See [modbus/AGENTS.md](modbus/AGENTS.md).

---

## Directory Structure

```
├── main.cpp                      # Entry point: creates web_server, calls start(8085)
├── CMakeLists.txt                # FetchContent: httplib, json; pkg-config: libmodbus
├── AGENTS.md                     # This file — master orchestrator
├── README.md                     # Build and run instructions
├── docs/                         # ← docs/AGENTS.md
│   ├── AGENTS.md
│   ├── PROJECT_WORKFLOW.md
│   └── MODBUS_WORKFLOW.md
├── include/                      # ← include/AGENTS.md
│   ├── AGENTS.md
│   ├── driver_base.hpp
│   ├── driver_registry.hpp
│   ├── pump_factory.hpp
│   ├── web_server.hpp
│   ├── model_info.hpp
│   └── simulation_variable.hpp
├── modbus/                       # ← modbus/AGENTS.md
│   ├── AGENTS.md
│   ├── modbus_types.hpp
│   ├── modbus_core.hpp
│   ├── protocol_writer.hpp
│   ├── channel_device.hpp
│   ├── modbus_server.hpp
│   └── modbus_server.cpp
├── simulation_kernel/            # ← simulation_kernel/AGENTS.md
│   ├── AGENTS.md
│   ├── driver_workflow.txt
│   ├── grundfos_pump.hpp / .cpp
│   └── wilo_pump.hpp / .cpp
└── static/
    └── index.html
```

---

## REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Web dashboard |
| GET | `/api/drivers` | List registered drivers |
| GET | `/api/drivers/{id}/models` | Models with comm protocol info |
| POST | `/api/simulation/start` | Start simulation — see body below |
| POST | `/api/simulation/stop` | Stop running simulation |
| GET | `/api/simulation/status` | Live sensor values + state |

### `POST /api/simulation/start` body

```json
{
  "driver_id": 1,
  "model_id":  0,
  "interface": "ethernet"
}
```

| Field | Required | Values | Default |
|-------|----------|--------|---------|
| `driver_id` | yes | integer (see `/api/drivers`) | — |
| `model_id` | yes | integer (see `/api/drivers/{id}/models`) | — |
| `interface` | no | `"ethernet"` / `"rs485"` | `"ethernet"` |
| `device` | only for rs485 | serial port path, e.g. `"/dev/ttyUSB0"` | `"/dev/ttyUSB0"` |
| `baud` | only for rs485 | integer, e.g. `9600` | `9600` |

RS-485 is only accepted when the selected model has `SupportRS485 = true`. If not, the request is rejected with HTTP 400.

---

## Build and Run

```bash
mkdir -p build && cd build && cmake .. && cmake --build . && ./pump_simulation
```

- HTTP dashboard: `http://localhost:8085`
- Modbus TCP: `localhost:1502` (slave ID 1)

**Dependencies:** C++17, libmodbus (system), cpp-httplib v0.18.3 (fetched), nlohmann/json v3.11.3 (fetched), pthreads

---

## Current Limitations

- Simulation physics are simplified: max value + random noise, no pump curve modeling
- `auto_control`, `threshold_low`, `threshold_high` in `pumpProto` are unused
- No persistent storage of simulation history
- Single concurrent simulation only (one driver/model at a time)
