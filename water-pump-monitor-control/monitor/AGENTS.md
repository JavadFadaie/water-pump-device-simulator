# AGENTS.md — monitor/

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains everything related to **reading and observing** pump state. Nothing here writes to the simulator.

---

## Files

| File | Role |
|------|------|
| `monitor_loop.hpp/cpp`   | Polling thread — calls `ModbusReader` every `poll_interval_ms`, fans out to store/logger/alarms |
| `pump_data_store.hpp/cpp` | Thread-safe ring buffer of `PumpSnapshot` structs |
| `alarm_manager.hpp/cpp`  | Compares each snapshot against thresholds, emits `AlarmEvent` |
| `data_logger.hpp/cpp`    | Appends snapshots as CSV rows to `logs/pump_YYYYMMDD.csv` |

---

## MonitorLoop

The central polling thread. Owned by `main.cpp`.

```
MonitorLoop::run()  [thread]
  every poll_interval_ms:
    snapshot = reader_.readSnapshot(base_addr_)
    if snapshot.valid:
        store_.push(snapshot)
        logger_.append(snapshot)
        alarm_manager_.check(snapshot)
    else:
        log warning, increment miss counter
```

Constructor takes: `ModbusReader&`, `PumpDataStore&`, `AlarmManager&`, `DataLogger&`, `uint16_t base_addr`, `int poll_interval_ms`.

`start()` / `stop()` manage the internal `std::thread`.

---

## PumpDataStore

Ring buffer capped at `N` snapshots (default 3600 = 1 hour at 1 s poll).  
Thread-safe: read and write lock the same `std::mutex`.

```cpp
class PumpDataStore {
public:
    void push(const PumpSnapshot& s);
    std::vector<PumpSnapshot> getAll() const;       // full history
    PumpSnapshot getLatest() const;                 // most recent
    std::vector<PumpSnapshot> getSince(std::chrono::system_clock::time_point t) const;
};
```

---

## AlarmManager

Configured at startup from `config.json` alarm thresholds. Stateless — no debounce or latch in v1.

```cpp
struct AlarmThresholds {
    float flow_rate_max;
    float pressure_max;
    float pump_power_max;
    float water_level_max;
};

class AlarmManager {
public:
    void setThresholds(const AlarmThresholds& t);
    void check(const PumpSnapshot& s);  // emits AlarmEvent to registered callbacks
    void onAlarm(std::function<void(AlarmEvent)> cb);
};
```

`check()` fires a callback for every threshold breached in the snapshot. Caller (main.cpp) registers a callback that logs to stderr and optionally triggers a control action.

---

## DataLogger

Creates one CSV file per day in `log_dir/`. Opens the file in append mode; writes a header row on first open.

CSV columns:
```
timestamp_iso8601, flow_rate, pressure, pump_power, water_level, run_time, pump_on
```

`DataLogger::append(PumpSnapshot)` is called from the MonitorLoop thread. Uses an internal mutex so future code can also call it from other threads.
