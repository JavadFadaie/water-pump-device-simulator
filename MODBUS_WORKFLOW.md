# HOW SIMULATED DATA FLOWS INTO MODBUS REGISTERS — FULL WORKFLOW

This document explains the architecture and workflow for integrating simulated data into Modbus registers for the device simulator project. It provides a detailed breakdown of the components, data flow, and key design patterns to help extend or modify the system in the future.

---

## ARCHITECTURE OVERVIEW

### Key Classes and Relationships

```
DeviceSimulator (device class, inherits generic_device_t)
    │
    ├── mModbus : unique_ptr<ModbusServer>  (concrete: ModbusTcp or ModbusRtu)
    │     │
    │     └── mModbusCore : shared_ptr<ModbusCore>
    │           │
    │           └── inherits ProtocolWriter<RegisterType, uint16_t>
    │               - mHoldingRegisters : multimap<uint16_t, shared_ptr<ModbusRegister>>
    │               - mInputRegisters   : multimap<uint16_t, shared_ptr<ModbusRegister>>
    │
    └── simulator : unique_ptr<channel_device<RegisterType, uint16_t>>
          │
          ├── writer : ProtocolWriter<RegisterType, uint16_t>&  (reference to *mModbusCore)
          └── channels : map<string, extended_channel_info>
                maps "CH_FLOW_RATE" → {InputRegister, address=10001, unit_id, value, ...}
                maps "CH_PRESSURE" → {InputRegister, address=10002, unit_id, value, ...}
                maps "CH_TEMPERATURE" → {InputRegister, address=10003, unit_id, value, ...}
```

---

## STEP 1 — CREATE THE MODBUS SERVER AND REGISTER MAP

### Modbus Server Initialization

In the `DeviceSimulator` constructor:

```cpp
if (this->device_proto.iface == SLInterface::IF_ETH)
    mModbus = std::make_unique<ModbusTcp>(this->device_proto.address);
else
    mModbus = std::make_unique<ModbusRtu>(this->device_proto.address);
```

- `ModbusTcp` and `ModbusRtu` inherit from `ModbusServer`.
- Inside the `ModbusServer` constructor:

```cpp
ModbusServer::ModbusServer(uint8_t id)
{
    mModbusCore = std::make_shared<ModbusCore>();
    initMembers();
    mSlaveId.push_back(id);
}
```

This creates the `mModbusCore` object, which manages the register map.

### Register Block Allocation

In `initModbusRegisters`:

```cpp
mModbus->mModbusCore->addRegisterBlock(InputRegister, REG_FLOW_RATE, ReadAccess, SENSOR_DATA_SIZE);
mModbus->mModbusCore->addRegisterBlock(InputRegister, REG_PRESSURE, ReadAccess, SENSOR_DATA_SIZE);
mModbus->mModbusCore->addRegisterBlock(InputRegister, REG_TEMPERATURE, ReadAccess, SENSOR_DATA_SIZE);
```

Each `addRegisterBlock` creates a contiguous range of `ModbusRegister` objects in a multimap keyed by address.

---

## STEP 2 — CREATE THE CHANNEL DEVICE AND MAP NAMED CHANNELS

### Channel Device Initialization

```cpp
simulator = std::make_unique<channel_device<RegisterType, uint16_t>>(*mModbus->mModbusCore);
```

- `channel_device` takes a `ProtocolWriter<RegisterType, uint16_t>&` reference, which points to `mModbusCore`.

### Named Channel Registration

In `initSimulatorChannels`:

```cpp
simulator->addChannel("CH_FLOW_RATE", InputRegister, REG_FLOW_RATE);
simulator->addChannel("CH_PRESSURE", InputRegister, REG_PRESSURE);
simulator->addChannel("CH_TEMPERATURE", InputRegister, REG_TEMPERATURE);
```

Each `addChannel` call maps a human-readable channel name (e.g., `CH_FLOW_RATE`) to a register address and metadata.

---

## STEP 3 — PERIODIC TICK: SIMULATION VALUES ARRIVE

In `pluginFunction<SimulationStep>`:

```cpp
for (auto &dev : plant->m_devices)
{
    if (dev->get_device_proto().AutoValues)
        detail::updateAutoValues(dev, ...);
    dev->update_driver_values();
}
```

This updates the `device_proto` struct with simulation values and calls `update_driver_values()` on each device.

---

## STEP 4 — DEVICE WRITES VALUES INTO REGISTERS

In `DeviceSimulator::update_driver_values`:

```cpp
simulator->setRegisterValueFloat("CH_FLOW_RATE", static_cast<float>(flow_rate), 1.0f);
simulator->setRegisterValueFloat("CH_PRESSURE", static_cast<float>(pressure), 1.0f);
simulator->setRegisterValueFloat("CH_TEMPERATURE", static_cast<float>(temperature), 1.0f);

mModbus->mModbusCore->setRegisterValueFloat(InputRegister, REG_FLOW_RATE, flow_rate);
mModbus->mModbusCore->setRegisterValueFloat(InputRegister, REG_PRESSURE, pressure);
mModbus->mModbusCore->setRegisterValueFloat(InputRegister, REG_TEMPERATURE, temperature);
```

- **Path A**: Writes via `channel_device` for named channels.
- **Path B**: Writes directly to `ModbusCore` for auxiliary registers.

---

## STEP 5 — MODBUSCORE STORES THE VALUE

In `ModbusCore::setRegisterValueFloat`:

1. Converts the float to a 32-bit integer (IEEE 754 bit pattern).
2. Splits the 32-bit value across two 16-bit registers.
3. Updates the `mValue` field of the `ModbusRegister` objects.

---

## STEP 6 — MODBUS CLIENT READS VALUES BACK

When a Modbus client sends a request:

1. `ModbusTcp::process_packet` or `ModbusRtu::process_byte` validates the request.
2. `ModbusServer::executeRequest` dispatches the function code.
3. For `ReadInputRegister` (FC=04):
   - `ModbusCore::ReadInputRegisters` retrieves the register values.
   - Values are byte-swapped to big-endian format and sent back to the client.

---

## COMPLETE DATA FLOW DIAGRAM

```
pluginFunction<SimulationStep>()
    │
    ├── DeviceSimulator::update_driver_values()
    │       ├── simulator->setRegisterValueFloat("CH_FLOW_RATE", flow_rate, 1.0f)
    │       │       ├── channel_device::save_channel_and_set_register<float>()
    │       │       │       ├── save_channel<float>() → channel.info.value = flow_rate
    │       │       │       └── set_register<float>() → ModbusCore::setRegisterValueFloat()
    │       └── mModbus->mModbusCore->setRegisterValueFloat(InputRegister, REG_FLOW_RATE, val)
    │
    └── ModbusCore::setRegisterValueFloat()
            └── Updates mInputRegisters multimap

ModbusClient → ModbusServer::executeRequest() → ModbusCore::ReadInputRegisters()
```

---

## KEY DESIGN PATTERNS TO REPLICATE

1. **Layered Separation**: Transport → Protocol Dispatch → Register Storage → Channel Abstraction.
2. **Channel Device as an Adapter**: Maps domain-specific names to raw register addresses.
3. **ProtocolWriter Abstraction**: Decouples `channel_device` from `ModbusCore`.
4. **Two Write Paths**: Named channels for reporting; direct writes for auxiliary registers.
5. **Shared Register Map**: Single source of truth for simulation writes and Modbus reads.

---

This document should be updated as the Modbus integration evolves.