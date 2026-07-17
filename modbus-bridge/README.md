# Modbus Bridge

This Node.js service bridges between the C++ Modbus TCP simulator and a React UI.

## Features
- Connects to the C++ simulator via Modbus TCP
- Exposes REST API for pump control
- Exposes WebSocket for live pump data

## Usage

1. Install dependencies:
   ```bash
   npm install
   ```
2. Start the bridge:
   ```bash
   npm start
   ```

- REST API: http://localhost:3001/control
- WebSocket: ws://localhost:3002

Configure the Modbus register map in `src/modbusConfig.js` as needed.
