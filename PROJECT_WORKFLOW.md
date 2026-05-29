# Water Pump Simulator Project Workflow

This document provides a comprehensive overview of the workflow of the Water Pump Simulator project. It is designed to serve as a reference for understanding the current implementation and for extending the project in the future.

---

## 1. Project Overview

The Water Pump Simulator is a simulation tool that models the behavior of different water pump drivers and their associated models. It provides a web-based interface for users to select a driver and model, start a simulation, and view real-time data such as flow rate, pressure, and power.

### Key Components:
- **Frontend**: A web-based UI for user interaction.
- **Backend**: A C++ server that handles requests, manages drivers, and runs simulations.
- **Simulation Kernel**: The core logic for simulating pump behavior.
- **Driver Registry**: A mechanism for registering and managing pump drivers.

---

## 2. Workflow Overview

### A. Startup
1. The application starts with `main.cpp`, which initializes the `web_server`.
2. The `web_server` sets up HTTP routes and begins listening for requests.

### B. Driver and Model Selection
1. **Frontend**: The user selects a driver and model from dropdown menus.
2. **Backend**:
   - The `/api/drivers` endpoint returns a list of available drivers.
   - The `/api/drivers/{driver_id}/models` endpoint returns models for the selected driver.

### C. Simulation Start
1. The user clicks the `Start` button.
2. The frontend sends a `POST /api/simulation/start` request with the selected driver and model IDs.
3. The backend:
   - Creates the driver instance.
   - Initializes the driver with its models.
   - Starts the simulation in a separate thread.

### D. Real-Time Data Projection
1. The frontend polls the backend every second via `GET /api/simulation/status`.
2. The backend returns the current simulation data (e.g., flow rate, pressure, power).
3. The frontend updates the dashboard with the new data.

### E. Simulation Stop
1. The user clicks the `Stop` button.
2. The frontend sends a `POST /api/simulation/stop` request.
3. The backend stops the simulation thread and resets the state.

---

## 3. Key Files and Their Roles

### A. Frontend
- **`static/index.html`**: The main HTML file for the web interface.
- **JavaScript**: Handles user interactions and communicates with the backend.

### B. Backend
- **`main.cpp`**: Entry point of the application.
- **`web_server.hpp`**: Manages HTTP routes and server logic.
- **`driver_registry.hpp`**: Registers and manages drivers.
- **`driver_base.hpp`**: Base class for all drivers.

### C. Simulation Kernel
- **`simulation_kernel/grundfos_pump.hpp`**: Implementation of the Grundfos driver.
- **`simulation_kernel/wilo_pump.hpp`**: Implementation of the Wilo driver.
- **`simulation_variable.hpp`**: Defines the simulation data structure.

---

## 4. Extending the Project

### A. Adding a New Driver
1. Create a new driver class (e.g., `new_pump.hpp`) in the `simulation_kernel` directory.
2. Implement the `driver_base` interface.
3. Register the driver in `driver_registry.hpp` using the `REGISTER_PUMP_DRIVER` macro.

### B. Adding New Simulation Variables
1. Update `simulation_variable.hpp` to include the new variables.
2. Modify the `simulate_values()` function in `driver_base` to calculate the new variables.
3. Update the frontend to display the new variables.

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