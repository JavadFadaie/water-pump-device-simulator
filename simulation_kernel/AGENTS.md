# AGENTS.md — Simulation Kernel (Pump Drivers)

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains all concrete pump driver implementations. Each driver is a self-contained simulation unit that owns its Modbus stack, simulation thread, and register map.

---

## Files

| File | Role |
|------|------|
| `grundfos_pump.hpp` | Class declaration only — inherits `driver_base` |
| `grundfos_pump.cpp` | Register enum (base 100), self-registration, models, constructor, all method implementations |
| `wilo_pump.hpp` | Class declaration only — inherits `driver_base` |
| `wilo_pump.cpp` | Register enum (base 200), self-registration, models, constructor, all method implementations |
| `driver_workflow.txt` | Step-by-step walkthrough of a full driver lifecycle with function call traces |

---

## Self-Registration Pattern

Each `.cpp` contains a single static object that fires before `main()`:

```cpp
static volatile driver_registry::register_t reghelper({
    .name         = "Grundfos",
    .id           = DriverId::GRUNDFOS_PUMP,
    .add_model    = grundfos_pump::add_model,
    .new_instance = grundfos_pump::new_instance,
    .add_to_test  = nullptr
});
```

This calls `driver_registry::instance().register_driver(desc)` at static initialization. By the time `main()` runs, all drivers are already registered. Adding or removing a driver from `CMakeLists.txt` SOURCES is the only wiring change needed.

---

## Driver Contract (pure-virtual methods on `driver_base`)

| Method | Responsibility |
|--------|---------------|
| `set_devices()` | Populate `device_list` with `ModelInfo` entries |
| `initChannels()` | Register named channels on `channel_device` with their register addresses |
| `update_driver_value()` | Called each simulation tick — reads `pumpProto` and calls `writeSimulationToRegisters()` |
| `writeSimulationToRegisters()` | Maps `pumpProto` fields to Modbus registers via `channel_device` and `mModbusCore` |

---

## Simulation Loop (in `driver_base`)

```
start_simulation()
  → mModbus->start()          // binds port 1502, starts Modbus server thread
  → simulation thread starts:
      loop every 1 s:
        simulate_values()     // updates pumpProto fields (physics, noise)
        update_driver_value() // writes pumpProto → Modbus registers
```

---

## How to Add a New Driver

1. **Create `simulation_kernel/new_pump.hpp`**
   ```cpp
   #include "driver_base.hpp"
   #include "channel_device.hpp"

   class new_pump : public driver_base {
   public:
       new_pump();
       static void add_model(std::vector<ModelInfo>&);
       static std::unique_ptr<driver_base> new_instance();
   private:
       pumpProto pump_data_;
       std::unique_ptr<channel_device<RegisterType, uint16_t>> pump_channels;
       void set_devices() override;
       void initChannels() override;
       void update_driver_value() override;
       void writeSimulationToRegisters() override;
   };
   ```

2. **Create `simulation_kernel/new_pump.cpp`**
   - Define register addresses (use a unique base, e.g., 300)
   - Add `static volatile driver_registry::register_t` self-registration block
   - Constructor: `ModbusTcp(1)` → `addRegisterBlock(...)` → `channel_device` → `initChannels()`
   - Implement all four override methods

3. **Add `DriverId::NEW_PUMP = 3`** to `include/model_info.hpp`

4. **Add `simulation_kernel/new_pump.cpp`** to `SOURCES` in `CMakeLists.txt`

5. No other files need changes — web UI and Modbus server auto-discover the new driver.

---

## Register Address Allocation

| Driver | Base Address | Range Used |
|--------|-------------|------------|
| Grundfos | 100 | 100–110 |
| Wilo | 200 | 200–210 |
| Next driver | 300 | 300–… |
