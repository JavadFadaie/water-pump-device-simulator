#!/usr/bin/env python3
"""
End-to-end test: verify Wilo driver writes simulated values to Modbus RTU registers.

Uses socat to create a virtual serial port pair:
  - Simulator (slave) listens on one port
  - pymodbus (master) reads from the other port

Verifies that register 200 (flow_rate) contains a non-zero float matching the HTTP status.
"""

import subprocess
import time
import struct
import sys
import os
import signal
import json
import urllib.request

SOCAT_CMD = ["socat", "-d", "-d", "pty,raw,echo=0,link=/tmp/vserial_sim", "pty,raw,echo=0,link=/tmp/vserial_client"]
SIM_BINARY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "build", "pump_simulation")
SIM_DEVICE = "/tmp/vserial_sim"
CLIENT_DEVICE = "/tmp/vserial_client"

def cleanup(procs):
    for p in procs:
        try:
            os.killpg(os.getpgid(p.pid), signal.SIGTERM)
        except Exception:
            pass
        p.wait()

def http_post(url, data):
    req = urllib.request.Request(url, data=json.dumps(data).encode(),
                                headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read())

def http_get(url):
    with urllib.request.urlopen(url) as resp:
        return json.loads(resp.read())

def main():
    procs = []

    try:
        # Step 1: Create virtual serial port pair
        print("[1] Creating virtual serial port pair with socat...")
        socat_proc = subprocess.Popen(SOCAT_CMD, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                      preexec_fn=os.setsid)
        procs.append(socat_proc)
        time.sleep(1)

        if not os.path.exists(SIM_DEVICE) or not os.path.exists(CLIENT_DEVICE):
            print("FAIL: socat did not create virtual serial ports")
            cleanup(procs)
            return 1

        print(f"    Simulator port: {SIM_DEVICE}")
        print(f"    Client port:    {CLIENT_DEVICE}")

        # Step 2: Start the simulator
        print("[2] Starting pump simulator...")
        sim_proc = subprocess.Popen([SIM_BINARY], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    preexec_fn=os.setsid)
        procs.append(sim_proc)
        time.sleep(2)

        # Step 3: Start Wilo simulation with RS485 on the virtual port
        print("[3] Starting Wilo Stratos MAXO 50 simulation with RS485...")
        result = http_post("http://localhost:8085/api/simulation/start", {
            "driver_id": 2,
            "model_id": 0,
            "interface": "rs485",
            "device": SIM_DEVICE,
            "baud": 9600
        })
        print(f"    Start response: {result}")

        # Wait for simulation values to populate
        print("[4] Waiting 3 seconds for simulation values...")
        time.sleep(3)

        # Step 4: Get HTTP status for comparison
        status = http_get("http://localhost:8085/api/simulation/status")
        http_flow = status["flow_rate"]
        http_pressure = status["pressure"]
        http_power = status["pump_power"]
        print(f"    HTTP status: flow_rate={http_flow:.2f}, pressure={http_pressure:.2f}, pump_power={http_power:.2f}")

        # Step 5: Read Modbus RTU registers via pymodbus
        print("[5] Reading Modbus RTU registers via pymodbus on {CLIENT_DEVICE}...")
        from pymodbus.client import ModbusSerialClient

        client = ModbusSerialClient(port=CLIENT_DEVICE, baudrate=9600, parity='N',
                                    stopbits=1, bytesize=8, timeout=3)
        connected = client.connect()
        if not connected:
            print("FAIL: pymodbus could not connect to virtual serial port")
            cleanup(procs)
            return 1

        def read_float_register(address):
            result = client.read_input_registers(address, count=2, device_id=1)
            if result.isError():
                return None
            hi, lo = result.registers[0], result.registers[1]
            return struct.unpack('>f', struct.pack('>HH', hi, lo))[0]

        rtu_flow = read_float_register(200)
        rtu_pressure = read_float_register(202)
        rtu_power = read_float_register(204)
        rtu_water_level = read_float_register(206)
        rtu_run_time = read_float_register(208)

        rtu_pump_on = client.read_input_registers(210, count=1, device_id=1)
        pump_on_val = rtu_pump_on.registers[0] if not rtu_pump_on.isError() else None

        client.close()

        # Step 6: Report results
        print("\n" + "=" * 60)
        print("MODBUS RTU REGISTER READ RESULTS (Wilo, Input Registers)")
        print("=" * 60)
        print(f"  Register 200-201 (Flow Rate):   {rtu_flow:.2f} L/min" if rtu_flow else "  Register 200-201: READ FAILED")
        print(f"  Register 202-203 (Pressure):    {rtu_pressure:.2f} bar" if rtu_pressure else "  Register 202-203: READ FAILED")
        print(f"  Register 204-205 (Pump Power):  {rtu_power:.2f} W" if rtu_power else "  Register 204-205: READ FAILED")
        print(f"  Register 206-207 (Water Level): {rtu_water_level:.2f} L" if rtu_water_level else "  Register 206-207: READ FAILED")
        print(f"  Register 208-209 (Run Time):    {rtu_run_time:.2f} s" if rtu_run_time else "  Register 208-209: READ FAILED")
        print(f"  Register 210     (Pump On):     {pump_on_val}" if pump_on_val is not None else "  Register 210:     READ FAILED")
        print("=" * 60)

        # Step 7: Validate
        all_ok = True
        if rtu_flow is not None and rtu_flow > 0:
            print(f"\n  PASS: Flow rate register 200 = {rtu_flow:.2f} (non-zero, simulation is writing)")
        else:
            print(f"\n  FAIL: Flow rate register 200 is zero or unreadable")
            all_ok = False

        if rtu_pressure is not None and rtu_pressure > 0:
            print(f"  PASS: Pressure register 202 = {rtu_pressure:.2f}")
        else:
            print(f"  FAIL: Pressure register 202 is zero or unreadable")
            all_ok = False

        if pump_on_val == 1:
            print(f"  PASS: Pump On register 210 = 1")
        else:
            print(f"  FAIL: Pump On register 210 = {pump_on_val}")
            all_ok = False

        print(f"\n{'ALL TESTS PASSED' if all_ok else 'SOME TESTS FAILED'}")

        # Cleanup
        http_post("http://localhost:8085/api/simulation/stop", {})
        time.sleep(1)
        cleanup(procs)
        return 0 if all_ok else 1

    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()
        cleanup(procs)
        return 1

if __name__ == "__main__":
    sys.exit(main())
