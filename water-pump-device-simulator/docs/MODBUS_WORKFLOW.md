# HOW SIMULATED DATA FLOWS INTO MODBUS REGISTERS — FULL WORKFLOW

This document explains the architecture and workflow for integrating simulated data into Modbus registers for the water pump device simulator. Each driver owns its own Modbus stack and defines its own register map.

---

## ARCHITECTURE OVERVIEW

### Key Classes and Relationships

```
grundfos_pump / wilo_pump (inherits driver_base)
    │
    ├── mModbus : unique_ptr<ModbusTcp>  (created in driver constructor)
    │     │
    │     └── mModbusCore : shared_ptr<ModbusCore>
    │           │
    │           └── inherits ProtocolWriter<RegisterType, uint16_t>
    │               - mHoldingRegisters : multimap<uint16_t, shared_ptr<ModbusRegister>>
    │               - mInputRegisters   : multimap<uint16_t, shared_ptr<ModbusRegister>>
    │
    └── pump_channels : unique_ptr<channel_device<RegisterType, uint16_t>>
          │
          ├── writer : ProtocolWriter<RegisterType, uint16_t>&  (reference to *mModbusCore)
          └── channels : map<string, extended_channel_info>
                maps "CH_FLOW_RATE"   → {InputRegister, address=100 or 200, ...}
                maps "CH_PRESSURE"    → {InputRegister, address=102 or 202, ...}
                maps "CH_PUMP_POWER"  → {InputRegister, address=104 or 204, ...}
                maps "CH_WATER_LEVEL" → {InputRegister, address=106 or 206, ...}
                maps "CH_RUN_TIME"    → {InputRegister, address=108 or 208, ...}
                maps "CH_PUMP_ON"     → {InputRegister, address=110 or 210, ...}
```

Each driver defines its own register addresses in a private enum inside its `.cpp` file:
- Grundfos: registers at 100–110
- Wilo: registers at 200–210

---

## STEP 1 — DRIVER CONSTRUCTOR CREATES THE MODBUS SERVER AND REGISTER MAP

In each driver's constructor (e.g., `grundfos_pump::grundfos_pump()`):

```cpp
mModbus = std::make_unique<ModbusTcp>(1);

mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister,   REG_FLOW_RATE,   AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister,   REG_PRESSURE,    AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
// ... all 6 sensor values for both InputRegister and HoldingRegister

pump_channels = std::make_unique<channel_device<RegisterType, uint16_t>>(*mModbus->mModbusCore);
initChannels();
```

- `ModbusTcp(1)` creates a Modbus server with slave ID 1.
- `addRegisterBlock` allocates contiguous `ModbusRegister` objects in the register map.
- `channel_device` takes a reference to `mModbusCore` (which implements `ProtocolWriter`).
- `initChannels()` maps human-readable names to register addresses.

---

## STEP 2 — NAMED CHANNEL REGISTRATION

In `initChannels()` (defined per driver):

```cpp
pump_channels->addChannel("CH_FLOW_RATE",   RegisterType::InputRegister, REG_FLOW_RATE);
pump_channels->addChannel("CH_PRESSURE",    RegisterType::InputRegister, REG_PRESSURE);
pump_channels->addChannel("CH_PUMP_POWER",  RegisterType::InputRegister, REG_PUMP_POWER);
pump_channels->addChannel("CH_WATER_LEVEL", RegisterType::InputRegister, REG_WATER_LEVEL);
pump_channels->addChannel("CH_RUN_TIME",    RegisterType::InputRegister, REG_RUN_TIME);
pump_channels->addChannel("CH_PUMP_ON",     RegisterType::InputRegister, REG_PUMP_ON);
```

Each `addChannel` call maps a human-readable name to a register type and address.

---

## STEP 3 — SIMULATION START AND PERIODIC TICK

When `start_simulation()` is called:

1. `mModbus->start()` — starts the Modbus TCP server on port 1502.
2. A simulation thread is created running `simulate_driver_values()`.

The thread loop runs every 1 second:
```
simulate_values()       → updates pumpProto with new sensor values
update_driver_value()   → calls writeSimulationToRegisters()
sleep 1 second
```

---

## STEP 4 — DRIVER WRITES VALUES INTO REGISTERS

In each driver's `writeSimulationToRegisters()`:

```cpp
// Path A: Write via channel_device (Input Registers)
pump_channels->setRegisterValueFloat("CH_FLOW_RATE",   pump.flow_rate,     1.0f);
pump_channels->setRegisterValueFloat("CH_PRESSURE",    pump.pressure,      1.0f);
pump_channels->setRegisterValueFloat("CH_PUMP_POWER",  pump.pump_power,    1.0f);
pump_channels->setRegisterValueFloat("CH_WATER_LEVEL", pump.water_level,   1.0f);
pump_channels->setRegisterValueFloat("CH_RUN_TIME",    pump.pump_run_time, 1.0f);
pump_channels->setRegisterValue("CH_PUMP_ON", pump.pump_on ? 1 : 0);

// Path B: Write directly to ModbusCore (Holding Registers)
mModbus->mModbusCore->setRegisterValueFloat(RegisterType::HoldingRegister, REG_FLOW_RATE,   pump.flow_rate);
mModbus->mModbusCore->setRegisterValueFloat(RegisterType::HoldingRegister, REG_PRESSURE,    pump.pressure);
// ... etc.
```

- **Path A**: Writes via `channel_device` to Input Registers (named channels).
- **Path B**: Writes directly to `ModbusCore` for Holding Registers.

---

## STEP 5 — MODBUSCORE STORES THE VALUE

In `ModbusCore::setRegisterValueFloat`:

1. Converts the float to a 32-bit integer (IEEE 754 bit pattern).
2. Splits the 32-bit value across two 16-bit registers (high word first).
3. Updates the `mValue` field of the `ModbusRegister` objects.

---

## STEP 6 — MODBUS CLIENT READS VALUES BACK

When a Modbus client (e.g., pymodbus, SCADA system) sends a request:

1. `ModbusTcp::process_packet` validates the TCP/Modbus frame.
2. `ModbusServer::executeRequest` dispatches the function code.
3. For `ReadInputRegister` (FC=04) or `ReadHoldingRegister` (FC=03):
   - `ModbusCore::ReadInputRegisters` or `ReadHoldingRegisters` retrieves the register values.
   - Values are byte-swapped to big-endian format and sent back to the client.

---

## COMPLETE DATA FLOW DIAGRAM

```
driver_base::simulate_driver_values() [thread loop, every 1s]
    │
    ├── simulate_values()  →  updates pumpProto (flow_rate, pressure, ...)
    │
    └── update_driver_value()
            └── writeSimulationToRegisters()
                    ├── pump_channels->setRegisterValueFloat("CH_FLOW_RATE", val, scale)
                    │       └── channel_device → ModbusCore::setRegisterValueFloat()
                    │                                └── Updates mInputRegisters
                    │
                    └── mModbus->mModbusCore->setRegisterValueFloat(HoldingRegister, addr, val)
                                                 └── Updates mHoldingRegisters

ModbusClient → ModbusTcp::process_packet() → ModbusServer::executeRequest()
                                               → ModbusCore::ReadInputRegisters()
                                               → ModbusCore::ReadHoldingRegisters()
```

---

## DRIVER LIFECYCLE

```
1. Static init:  driver_registry::register_t registers the driver
2. User starts:  web_server creates driver via driver_registry::create_driver(id)
3. Constructor:  driver creates ModbusTcp + channel_device + register blocks
4. start_simulation():  starts Modbus TCP server (port 1502) + simulation thread
5. Running:      simulation loop writes values to registers every 1s
6. stop_simulation():   stops simulation thread + Modbus TCP server (releases port 1502)
7. Driver switch: old driver is destroyed, new driver created (step 2 again)
```

---

## KEY DESIGN PATTERNS

1. **Driver-Owned Modbus**: Each driver creates and owns its Modbus stack — no shared infrastructure.
2. **Layered Separation**: Transport → Protocol Dispatch → Register Storage → Channel Abstraction.
3. **Channel Device as Adapter**: Maps domain-specific names to raw register addresses.
4. **ProtocolWriter Abstraction**: Decouples `channel_device` from `ModbusCore`.
5. **Two Write Paths**: Named channels for Input Registers; direct writes for Holding Registers.
6. **Port Lifecycle**: `stop_simulation()` releases port 1502 before a new driver can bind.

---

This document should be updated as the Modbus integration evolves.
