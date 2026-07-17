module.exports = {
  host:    '127.0.0.1',   // C++ simulator IP
  port:    502,           // Modbus TCP default port
  unitId:  1,             // matches device_config.address
  registers: {
    FLOW_RATE:   { address: 100, type: 'INPUT',   size: 2, encoding: 'float32' },
    PRESSURE:    { address: 102, type: 'INPUT',   size: 2, encoding: 'float32' },
    RPM:         { address: 104, type: 'INPUT',   size: 2, encoding: 'float32' },
    RUN_HOURS:   { address: 106, type: 'INPUT',   size: 2, encoding: 'float32' },
    MOTOR_POWER: { address: 108, type: 'INPUT',   size: 2, encoding: 'float32' },
    VOLTAGE:     { address: 110, type: 'INPUT',   size: 2, encoding: 'float32' },
    CURRENT:     { address: 112, type: 'INPUT',   size: 2, encoding: 'float32' },
    PUMP_ON_OFF: { address: 200, type: 'HOLDING', size: 1, encoding: 'uint16'  },
    TARGET_RPM:  { address: 201, type: 'HOLDING', size: 1, encoding: 'uint16'  }
  }
};
