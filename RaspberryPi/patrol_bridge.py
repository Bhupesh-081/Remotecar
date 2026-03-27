#!/usr/bin/env python3
"""Bridge ESP32 patrol telemetry to MQTT for monitoring and logging."""

from __future__ import annotations

import argparse
import json
import time
from datetime import datetime, timezone

import paho.mqtt.client as mqtt
import requests


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="ESP32 patrol telemetry bridge")
    parser.add_argument("--host", default="192.168.4.1", help="ESP32 host or IP")
    parser.add_argument("--mqtt-host", default="localhost", help="MQTT broker host")
    parser.add_argument("--mqtt-port", default=1883, type=int, help="MQTT broker port")
    parser.add_argument("--period", default=1.0, type=float, help="Polling interval seconds")
    return parser.parse_args()


def make_health_event(message: str) -> str:
    payload = {
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "message": message,
    }
    return json.dumps(payload)


def main() -> None:
    args = parse_args()
    base_url = f"http://{args.host}"

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.connect(args.mqtt_host, args.mqtt_port, keepalive=30)
    client.loop_start()

    print(f"Polling {base_url}/api/status every {args.period}s")

    while True:
        try:
            response = requests.get(f"{base_url}/api/status", timeout=2.5)
            response.raise_for_status()
            status = response.json()

            client.publish("patrolcar/status/raw", json.dumps(status), qos=0)
            client.publish("patrolcar/status/battery", str(status.get("battery_pct", -1)), qos=0)
            client.publish("patrolcar/status/distance", str(status.get("distance_cm", -1)), qos=0)
            client.publish("patrolcar/events/health", make_health_event("status_ok"), qos=0)

            print("motion=", status.get("motion"), "distance=", status.get("distance_cm"), "battery=", status.get("battery_pct"))
        except Exception as exc:  # noqa: BLE001
            client.publish("patrolcar/events/health", make_health_event(f"status_error:{exc}"), qos=0)
            print("Bridge warning:", exc)

        time.sleep(max(0.2, args.period))


if __name__ == "__main__":
    main()
