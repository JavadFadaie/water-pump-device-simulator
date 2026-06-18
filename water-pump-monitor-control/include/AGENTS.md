# AGENTS.md — include/

**Parent:** [../AGENTS.md](../AGENTS.md)

Shared types used across `modbus/`, `monitor/`, and `control/`. No implementation here — headers only.

---

## Files

| File | Role |
|------|------|
| `pump_snapshot.hpp`  | One timestamped sensor reading (the unit of data flow) |
| `alarm_event.hpp`    | Alarm event struct + severity enum |
| `pump_config.hpp`    | Runtime configuration loaded from `config.json` |
| `driver_map.hpp`     | Maps driver_id → Modbus base address |

---

## PumpSnapshot

```cpp
struct PumpSnapshot {
    std::chrono::system_clock::time_point timestamp;
    float  flow_rate   = 0.0f;   // L/min
    float  pressure    = 0.0f;   // bar
    float  pump_power  = 0.0f;   // W
    float  water_level = 0.0f;   // L
    float  run_time    = 0.0f;   // s
    bool   pump_on     = false;
    bool   valid       = false;  // false if Modbus read failed
};
```

`valid = false` means the read failed (connection lost, timeout). Consumers must check this before using the values.

---

## AlarmEvent

```cpp
enum class AlarmLevel { WARNING, CRITICAL };

struct AlarmEvent {
    std::chrono::system_clock::time_point timestamp;
    AlarmLevel level;
    std::string channel;    // e.g. "pressure", "flow_rate"
    float       value;      // the value that breached the threshold
    float       threshold;  // the configured limit
};
```

---

## PumpConfig

Loaded once at startup from `config.json`. The `transport` field selects which sub-struct is used.

```cpp
enum class TransportType { TCP, RTU };

struct TcpConfig {
    std::string host     = "127.0.0.1";
    int         port     = 1502;
    int         slave_id = 1;
};

struct RtuConfig {
    std::string device    = "/dev/ttyUSB0";
    int         baud      = 9600;
    char        parity    = 'N';   // 'N', 'E', 'O'
    int         data_bits = 8;
    int         stop_bits = 1;
    int         slave_id  = 1;
};

struct AlarmThresholds {
    float flow_rate_max   = 60.0f;
    float pressure_max    = 4.0f;
    float pump_power_max  = 600.0f;
    float water_level_max = 100.0f;
};

struct PumpConfig {
    TransportType transport      = TransportType::TCP;
    TcpConfig     tcp;
    RtuConfig     rtu;
    int           rest_port      = 8085;   // TCP mode only; ignored for RTU
    int           driver_id      = 1;
    int           model_id       = 0;
    int           poll_interval_ms = 1000;
    std::string   log_dir        = "logs/";
    AlarmThresholds alarms;
};

PumpConfig load_config(const std::string& path); // reads config.json via nlohmann::json
```

---

## DriverMap

Maps `driver_id` (from `GET /api/drivers`) to the Modbus base address used in the simulator.

```cpp
// Returns the input register base address for a given driver_id.
// Grundfos = 100, Wilo = 200. Returns -1 for unknown driver.
int base_address_for_driver(int driver_id);
```

Kept in one place so that adding a new driver (base address 300+) only requires editing this file.
