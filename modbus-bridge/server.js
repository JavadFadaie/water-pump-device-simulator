const ModbusRTU  = require('modbus-serial');
const express    = require('express');
const { Server } = require('ws');
const config     = require('./src/modbusConfig');

const client = new ModbusRTU();

async function startBridge() {
  // Connect to C++ simulator
  await client.connectTCP(config.host, { port: config.port });
  client.setID(config.unitId);

  // Start HTTP API server (React fetches from here)
  const app = express();
  app.use(express.json());
  app.listen(3001, () => console.log('REST API listening on port 3001'));

  // Control endpoint
  app.post('/control', async (req, res) => {
    const { command, value } = req.body;
    try {
      if (command === 'PUMP_ON_OFF') {
        await client.writeRegister(config.registers.PUMP_ON_OFF.address, value ? 1 : 0);
      } else if (command === 'TARGET_RPM') {
        await client.writeRegister(config.registers.TARGET_RPM.address, Math.round(value));
      }
      res.json({ success: true });
    } catch (err) {
      res.status(500).json({ error: err.message });
    }
  });

  // Start WebSocket server (React subscribes for live updates)
  const wss = new Server({ port: 3002 });
  startPollingLoop(wss);
}

function decodeRegisters(regs) {
  const buf = Buffer.alloc(regs.length * 2);
  regs.forEach((v, i) => buf.writeUInt16BE(v, i * 2));
  return {
    flowRate:   buf.readFloatBE(0),   // regs 100-101
    pressure:   buf.readFloatBE(4),   // regs 102-103
    rpm:        buf.readFloatBE(8),   // regs 104-105
    runHours:   buf.readFloatBE(12),  // regs 106-107
    motorPower: buf.readFloatBE(16),  // regs 108-109
    voltage:    buf.readFloatBE(20),  // regs 110-111
    current:    buf.readFloatBE(24),  // regs 112-113
  };
}

async function startPollingLoop(wss) {
  setInterval(async () => {
    try {
      // Read Input Registers: addresses 100–113 (7 float32 values = 14 regs)
      const data = await client.readInputRegisters(100, 14);
      const payload = decodeRegisters(data.data);
      wss.clients.forEach(ws => {
        if (ws.readyState === ws.OPEN)
          ws.send(JSON.stringify({ type: 'PUMP_DATA', payload }));
      });
    } catch (err) {
      console.error('Modbus read error:', err.message);
    }
  }, 1000);
}

startBridge().catch(err => {
  console.error('Failed to start Modbus bridge:', err);
  process.exit(1);
});
