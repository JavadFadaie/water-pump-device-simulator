# AGENTS.md — Core Infrastructure (`include/`)

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains the shared infrastructure that glues the whole application together. These headers define the contracts that all drivers must implement and the runtime that manages them.

---

## Files

| File | Role |
|------|------|
| `model_info.hpp` | `DriverId` enum, `CommProtocol` enum, `DeviceType` enum, `ModelInfo` struct |
| `simulation_variable.hpp` | `pumpProto` struct — all sensor fields (flow rate, pressure, power, water level, run time, control flags), zero-initialized |
| `driver_base.hpp` | Abstract base class for all pump drivers; owns `mModbus`, simulation thread, atomic state |
| `driver_registry.hpp` | Singleton registry; stores `driver_descriptor` map; provides `register_driver()`, `create_driver()`, `list_drivers()`, `get_models()` |
| `web_server.hpp` | Full HTTP server (cpp-httplib); all REST routes; manages one active `driver_base` instance |

---

## `driver_base` — Abstract Driver Contract

```
driver_base owns:
  mModbus     : unique_ptr<ModbusServer>     TCP or RTU server, started/stopped with simulation
  pump        : pumpProto&                   reference to driver's sensor data struct
  device_list : vector<ModelInfo>            populated by set_devices()

Pure-virtual methods (each driver must implement):
  set_devices()                 → fill device_list
  initChannels()                → wire channel names to register addresses
  update_driver_value()         → called every tick; reads pumpProto, calls writeSimulationToRegisters()
  writeSimulationToRegisters()  → maps pumpProto fields to Modbus registers

Public methods (provided by driver_base):
  start_simulation()            → starts Modbus server + simulation thread
  stop_simulation()             → stops both, joins thread
  configureRtu(device, baud)    → hot-swaps the active TCP server for ModbusRtu while keeping
                                   the shared ModbusCore intact — register state is preserved;
                                   must be called before start_simulation()
  is_running()                  → atomic bool
  get_pump_data()               → returns pumpProto snapshot
  get_mutex()                   → mutex for thread-safe reads
```

---

## `driver_registry` — Self-Registration Singleton

Drivers call `driver_registry::instance().register_driver(desc)` at static initialization via the `register_t` helper. The registry never needs to be edited manually.

```cpp
struct driver_descriptor {
    std::string name;
    DriverId    id;
    void (*add_model)(std::vector<ModelInfo>&);
    std::unique_ptr<driver_base> (*new_instance)();
    void (*add_to_test)(void*);   // optional, nullable
};
```

`create_driver(id)` calls `new_instance()` and returns the resulting `unique_ptr<driver_base>`.

---

## `model_info.hpp` — Extending the Type System

When adding a new driver, add its `DriverId` here:

```cpp
enum class DriverId {
    GRUNDFOS_PUMP = 1,
    WILO_PUMP     = 2,
    NEW_PUMP      = 3,   // ← add here
};
```

`CommProtocol` lists supported transports (`ModbusTCP`, `ModbusRTU`, `Both`). `ModelInfo` carries the display name, device type, and supported protocols for each model — used by the web API and UI dropdown.

---

## `web_server` — REST API

The web server holds a single `unique_ptr<driver_base> active_driver_`. All REST handlers are in `web_server.hpp`. It calls `driver_registry` directly for discovery and creation.

See [../AGENTS.md](../AGENTS.md) for the full endpoint table.

---

