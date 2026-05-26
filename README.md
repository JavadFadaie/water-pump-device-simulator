# Water Pump Device Simulator

A C++ simulation engine for water pump devices with a web UI and REST API. Supports multiple pump manufacturers (Grundfos, Wilo) through an extensible driver architecture using factory and strategy patterns.

## Features

- **Web UI** for real-time monitoring and control from a browser
- Simulates real-time pump parameters: flow rate, pressure, power consumption, water level
- Multi-driver support with pluggable pump manufacturer drivers
- REST API for remote control and monitoring via cpp-httplib
- Threaded simulation engine with thread-safe data access
- Device catalog per driver with selectable pump models

## Project Structure

```
water-pump-device-simulator/
├── main.cpp                        # Entry point, Crow HTTP server and API routes
├── CMakeLists.txt                  # CMake build configuration
├── static/
│   └── index.html                  # Web UI (single-page application)
├── include/
│   ├── driver_base.hpp             # Abstract base class for pump drivers
│   ├── pump_factory.hpp            # Factory for creating pump driver instances
│   ├── simulation_variable.hpp     # Data structures (pumpProto, device_properties)
│   ├── driver_registery.hpp        # Driver registry metadata
│   ├── driverId.hpp                # Driver ID enum
│   ├── manufactureId.hpp           # Manufacturer ID enum and lookup table
│   └── third_party/               # cpp-httplib and nlohmann/json headers
├── simulation_kernel/
│   ├── grundfos_pump.hpp           # Grundfos pump driver implementation
│   └── wilo_pump.hpp              # Wilo pump driver implementation
└── build/                          # Build output directory
```

## Prerequisites

- C++17 compatible compiler (GCC, Clang)
- CMake 3.10+
- pthreads

## Build and Run

```bash
cd build
cmake ..
make
./pump_simulation
```

The server starts on `http://localhost:18080`.

## Web UI

Open `http://localhost:18080` in your browser to access the web interface.

The web UI provides:
- **Driver selection** — choose between Grundfos and Wilo pump drivers
- **Model selection** — pick a specific pump model and view its specifications
- **Start/Stop controls** — start and stop the simulation with a single click
- **Live dashboard** — real-time display of flow rate, pressure, pump power, water level, and run time with visual gauges
- **Activity log** — timestamped log of all actions and events

## API Endpoints

| Method | Endpoint                     | Description                        |
|--------|------------------------------|------------------------------------|
| GET    | `/`                          | Web UI                             |
| GET    | `/api/hello`                 | Health check                       |
| GET    | `/api/drivers`               | List available pump drivers        |
| GET    | `/api/drivers/<id>/models`   | List models for a specific driver  |
| POST   | `/api/simulation/start`      | Start a simulation                 |
| POST   | `/api/simulation/stop`       | Stop the running simulation        |
| GET    | `/api/simulation/status`     | Get current simulation status      |

### Start Simulation

```bash
curl -X POST http://localhost:18080/api/simulation/start \
  -H "Content-Type: application/json" \
  -d '{"driver_id": 1, "model_id": 0}'
```

The `model_id` is a 0-based index into the model list returned by `/api/drivers/<id>/models`.

### Check Status

```bash
curl http://localhost:18080/api/simulation/status
```

### Stop Simulation

```bash
curl -X POST http://localhost:18080/api/simulation/stop
```

## Supported Drivers

| ID | Manufacturer | Models                              |
|----|-------------|--------------------------------------|
| 1  | Grundfos    | CRE 20, CRE 30                      |
| 2  | Wilo        | Stratos MAXO 50, Stratos MAXO 60    |

## Architecture

The simulator uses a **factory + strategy pattern**:

1. `driver_base` defines the simulation loop and common behavior
2. Concrete drivers (`grundfos_pump`, `wilo_pump`) implement device catalogs and custom update logic
3. `pump_factory` instantiates the correct driver based on a driver ID
4. The cpp-httplib server exposes the simulation through REST endpoints and serves the web UI
