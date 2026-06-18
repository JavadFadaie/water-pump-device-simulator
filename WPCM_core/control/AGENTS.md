# AGENTS.md — control/

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains everything related to **sending commands** to the simulator. Nothing here reads sensor data.

---

## Files

| File | Role |
|------|------|
| `control_client.hpp/cpp`   | Public control API — `start()`, `stop()`, `setFlowSetpoint(float)` |
| `http_client.hpp/cpp`      | Thin wrapper around cpp-httplib for simulator REST calls |
| `setpoint_manager.hpp/cpp` | Holds the desired state; detects changes; triggers Modbus writes |

---

## ControlClient

The single entry point for all control actions. `main.cpp` and `AlarmManager` callbacks use only this class.

```cpp
class ControlClient {
public:
    ControlClient(HttpClient& http, ModbusWriter& writer, SetpointManager& sp);

    // Lifecycle — calls simulator REST API
    bool startSimulation(int driver_id, int model_id);
    bool stopSimulation();

    // Setpoints — queued through SetpointManager, written via ModbusWriter
    void setFlowSetpoint(float l_per_min);
    void setPumpOn(bool on);

    // Read back current desired state (does NOT poll simulator)
    const SetpointState& currentSetpoints() const;
};
```

---

## HttpClient

Wraps `httplib::Client` for the two REST endpoints needed for lifecycle control.

```cpp
class HttpClient {
public:
    HttpClient(const std::string& host, int port);

    bool post(const std::string& path, const nlohmann::json& body);
    nlohmann::json get(const std::string& path);
};
```

Used by `ControlClient` for:
- `POST /api/simulation/start` — `{"driver_id": N, "model_id": M}`
- `POST /api/simulation/stop`

---

## SetpointManager

Tracks the desired state. On each `apply()` call it compares desired vs last-sent and calls `ModbusWriter` only when something changed.

```cpp
struct SetpointState {
    float  flow_setpoint  = 0.0f;
    bool   pump_on        = false;
};

class SetpointManager {
public:
    void setFlow(float l_per_min);
    void setPumpOn(bool on);
    void apply(ModbusWriter& writer); // writes only changed values
};
```

`apply()` is called from `ControlClient` after every setter. The Modbus write addresses follow the planned holding register map defined in [modbus/AGENTS.md](../modbus/AGENTS.md).

---

## Control Paths Summary

```
ControlClient::startSimulation()
  → HttpClient::post("/api/simulation/start", {driver_id, model_id})
  → simulator starts, Modbus server binds port 1502

ControlClient::setFlowSetpoint(45.0f)
  → SetpointManager::setFlow(45.0f)
  → SetpointManager::apply()
      → ModbusWriter::writeFloat(502, 45.0f)   [holding register 502-503]

AlarmCallback: pressure exceeded
  → ControlClient::setPumpOn(false)
      → SetpointManager::setPumpOn(false)
      → ModbusWriter::writeUint16(500, 0)       [holding register 500]
```

---

## Note on Simulator Extension

The Modbus write path (FC16) currently targets holding registers that **do not yet exist** in the simulator. Until the simulator is extended with writable holding registers, `ControlClient::setFlowSetpoint()` and `setPumpOn()` will log a "write not yet supported" warning and no-op. Lifecycle control (start/stop via REST) works immediately.
