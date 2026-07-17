export async function setPumpOnOff(on: boolean) {
  await fetch('http://localhost:3001/control', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ command: 'PUMP_ON_OFF', value: on }),
  });
}

export async function setTargetRpm(rpm: number) {
  await fetch('http://localhost:3001/control', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ command: 'TARGET_RPM', value: rpm }),
  });
}
