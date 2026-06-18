# AGENTS.md — Documentation (`docs/`)

**Parent:** [../AGENTS.md](../AGENTS.md)

This folder contains human-readable documentation for the project. These are reference documents — they do not affect the build.

---

## Files

| File | Contents |
|------|----------|
| `PROJECT_WORKFLOW.md` | Comprehensive workflow overview: how the application starts, how drivers are registered, how the simulation runs end-to-end. Use this to understand the full lifecycle before making structural changes. |
| `MODBUS_WORKFLOW.md` | Detailed walkthrough of how simulated sensor data flows from `pumpProto` fields through `channel_device` into `ModbusCore` registers and out to a Modbus TCP client. Use this when debugging register values or adding new channels. |

---

## Related

- Architecture and design patterns → [../AGENTS.md](../AGENTS.md)
- Modbus protocol internals → [../modbus/AGENTS.md](../modbus/AGENTS.md)
- Adding a new driver (step-by-step) → [../simulation_kernel/AGENTS.md](../simulation_kernel/AGENTS.md)
