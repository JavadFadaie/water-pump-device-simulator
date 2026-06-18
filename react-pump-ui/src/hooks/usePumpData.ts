import { useEffect, useState } from 'react';

export interface PumpData {
  flowRate:   number;
  pressure:   number;
  rpm:        number;
  runHours:   number;
  motorPower: number;
  voltage:    number;
  current:    number;
}

export function usePumpData(): PumpData | null {
  const [data, setData] = useState<PumpData | null>(null);

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:3002');
    ws.onmessage = (evt) => {
      const msg = JSON.parse(evt.data);
      if (msg.type === 'PUMP_DATA') setData(msg.payload);
    };
    return () => ws.close();
  }, []);

  return data;
}
