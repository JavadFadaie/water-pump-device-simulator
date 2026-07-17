# Water Pump Device Simulator (SMCS_AWP)

Industrial water pump device simulator written in C++17. Simulates pump sensor data and exposes it through an HTTP web dashboard (port 8085) and a Modbus TCP server (port 1502). Designed for testing SCADA/PLC integration without physical pump hardware.

## Features

- **Multi-driver support**: Grundfos (2 models) and Wilo (2 models), extensible to new manufacturers
- **Real-time simulation**: Flow rate, pressure, pump power, water level, run time, pump on/off — updated every 1 second
- **Web dashboard**: Dark-themed UI with driver/model selection, live sensor cards, animated gauges
- **Modbus TcCP**: Eah driver owns its Modbus stack with driver-specific register addresses (IEEE 754 float encoding)
- **Self-registering drivers**: New drivers auto-discover — no changes to main, web server, or Modbus code

## Build

```bash
mkdir -p build && cd build && cmake .. && cmake --build .
```

### Dependencies

- C++17 compiler (GCC/Clang)
- libmodbus (system-installed, found via pkg-config)
- cpp-httplib v0.18.3 (fetched by CMake)
- nlohmann/json v3.11.3 (fetched by CMake)

## Run

```bash
cd build && ./pump_simulation
```

- Web UI: http://localhost:8085
- Modbus TCP: localhost:1502 (slave ID 1)

## Architecture

Each pump driver (Grundfos, Wilo) is a self-contained unit that:
1. Registers itself at static initialization via `driver_descriptor` + `register_t`
2. Creates its own `ModbusTcp`, `ModbusCore`, and `channel_device` in its constructor
3. Starts both the Modbus TCP server and simulation thread on `start_simulation()`
4. Writes simulated values to its own register map every second

```
main.cpp → web_server (HTTP API)
             └── driver_registry → driver_base
                                     └── grundfos_pump / wilo_pump
                                           ├── ModbusTcp (port 1502)
                                           └── channel_device (named channels → registers)
```

## Documentation

- `AGENTS.md` — Full architecture, directory structure, how to add a driver
- `docs/PROJECT_WORKFLOW.md` — Project workflow and extension guide
- `docs/MODBUS_WORKFLOW.md` — Modbus data flow from simulation to registers
- `simulation_kernel/driver_workflow.txt` — Step-by-step example walkthrough with function calls
- `SMCS_AWP.txt` — Project notes and implementation history
