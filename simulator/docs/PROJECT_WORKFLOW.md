# Water Pump Simulator Project Workflow

This document provides a comprehensive overview of the workflow of the Water Pump Simulator project. It is designed to serve as a reference for understanding the current implementation and for extending the project in the future.

---

## 1. Project Overview

The Water Pump Simulator is a simulation tool that models the behavior of different water pump drivers and their associated models. It provides a web-based interface for users to select a driver and model, start a simulation, and view real-time data such as flow rate, pressure, and power. Each driver owns its own Modbus TCP server, exposing simulated values to external SCADA/PLC clients.

### Key Components:
- **Frontend**: A web-based UI for user interaction.
- **Backend**: A C++ server that handles HTTP requests, manages drivers, and runs simulations.
- **Simulation Kernel**: Per-driver classes that simulate pump behavior and write values to Modbus registers.
- **Driver Registry**: A self-registering system — drivers register themselves at static initialization via `driver_descriptor` and `register_t`.

---

## 2. Workflow Overview

### A. Startup
1. The application starts with `main.cpp`, which creates a `web_server` and starts it on port 8085.
2. During static initialization (before `main()` runs), each driver's `static volatile driver_registry::register_t` executes and registers itself with the singleton `driver_registry`.
3. The `web_server` sets up HTTP routes and begins listening for requests.

### B. Driver and Model Selection
1. **Frontend**: The user selects a driver and model from dropdown menus.
2. **Backend**:
   - The `/api/drivers` endpoint calls `driver_registry::list_drivers()` to return all registered drivers.
   - The `/api/drivers/{driver_id}/models` endpoint calls `driver_registry::get_models(id)`, which invokes the driver's static `add_model()` function to return models with full specs (including communication protocol support).

### C. Simulation Start
1. The user clicks the `Start` button.
2. The frontend sends a `POST /api/simulation/start` request with `driver_id` and `model_id`.
3. The backend:
   - If an old simulation is running, calls `stop_simulation()` on it (stops sim thread + Modbus server).
   - Creates a new driver instance via `driver_registry::create_driver(id)`.
   - Calls `set_devices()` and `select_device_by_index(model_id)`.
   - Calls `start_simulation()`, which starts both the simulation thread AND the driver's Modbus TCP server on port 1502.

### D. Real-Time Data Projection
1. The frontend polls the backend every second via `GET /api/simulation/status`.
2. The backend reads the driver's `pumpProto` data and returns it as JSON.
3. The frontend updates the dashboard with the new data.

### E. Simulation Stop
1. The user clicks the `Stop` button.
2. The frontend sends a `POST /api/simulation/stop` request.
3. The backend calls `stop_simulation()`, which stops the simulation thread and the Modbus TCP server (releasing port 1502).

---

## 3. Key Files and Their Roles

### A. Frontend
- **`static/index.html`**: The main HTML file for the web interface (HTML + CSS + JS).

### B. Backend
- **`main.cpp`**: Entry point — creates `web_server` and starts it. No Modbus setup here.
- **`include/web_server.hpp`**: Manages HTTP routes and server logic. Handles driver lifecycle (create/start/stop).
- **`include/driver_registry.hpp`**: Singleton registry with `driver_descriptor` struct and `register_t` for self-registration.
- **`include/driver_base.hpp`**: Abstract base class — simulation loop, threading, owns `ModbusTcp` (protected member).
- **`include/model_info.hpp`**: `ModelInfo` struct, `CommProtocol`, `DeviceType`, `DriverId` enums.

### C. Simulation Kernel (per driver: header + .cpp)
- **`simulation_kernel/grundfos_pump.hpp`**: Grundfos class declaration only.
- **`simulation_kernel/grundfos_pump.cpp`**: Register enum (100+), static registration, model list, constructor (creates ModbusTcp + channel_device), all method implementations.
- **`simulation_kernel/wilo_pump.hpp`**: Wilo class declaration only.
- **`simulation_kernel/wilo_pump.cpp`**: Register enum (200+), static registration, model list, constructor, all method implementations.
- **`include/simulation_variable.hpp`**: Defines the `pumpProto` struct (all fields zero-initialized).

---

## 4. Extending the Project

### A. Adding a New Driver
1. Create `simulation_kernel/new_pump.hpp` — class declaration inheriting `driver_base`.
2. Create `simulation_kernel/new_pump.cpp` — contains:
   - Driver-specific register enum (unique address range)
   - `static volatile driver_registry::register_t` with `driver_descriptor`
   - `add_model()` with `ModelInfo` entries (including comm protocol fields)
   - Constructor: creates `ModbusTcp(1)`, adds register blocks, creates `channel_device`, calls `initChannels()`
   - All method implementations
3. Add `DriverId::NEW_PUMP` to `include/model_info.hpp`.
4. Add the `.cpp` to `CMakeLists.txt` sources.
5. No other files need changes — the web UI discovers drivers automatically.

### B. Adding New Simulation Variables
1. Update `simulation_variable.hpp` to include the new fields in `pumpProto`.
2. Modify the `simulate_values()` function in `driver_base` to calculate the new values.
3. Add new register addresses in the driver's `.cpp` register enum.
4. Add new register blocks and channels in the driver's constructor and `initChannels()`.
5. Write the new values in `writeSimulationToRegisters()`.
6. Update the frontend to display the new variables.

### C. Enhancing the Frontend
1. Modify `static/index.html` to add new UI elements.
2. Update the JavaScript logic to handle new features.

### D. Changing the Communication Protocol
1. Replace HTTP polling with WebSocket communication for real-time updates.
2. Update the `web_server` to handle WebSocket connections.
3. Modify the frontend to use WebSocket instead of HTTP polling.

---

## 5. Future Considerations

### A. Scalability
- Consider using a database to store simulation data for long-running simulations.
- Implement load balancing if the server needs to handle multiple users simultaneously.

### B. Testing
- Add unit tests for the simulation kernel and drivers.
- Use integration tests to verify the end-to-end workflow.

### C. Documentation
- Maintain up-to-date documentation for all new features.
- Use tools like Doxygen to generate API documentation.

---

This document should be updated as the project evolves to ensure it remains a reliable reference for future development.
