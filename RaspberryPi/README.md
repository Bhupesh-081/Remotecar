# Raspberry Pi Companion Node

This folder upgrades the vehicle from a standalone prototype to a distributed robotics system.

## What this node does

- Connects to the ESP32 patrol API over Wi-Fi
- Polls telemetry and forwards it to MQTT topics
- Stores mission logs for assessment and report writing
- Enables integration with dashboards such as Node-RED or Grafana

## Quick setup

1. Install Python 3.10+ on Raspberry Pi OS.
2. Install dependencies:

   pip install -r requirements.txt

3. Run bridge:

   python3 patrol_bridge.py --host 192.168.4.1 --mqtt-host localhost

## MQTT topics

- patrolcar/status/raw
- patrolcar/status/battery
- patrolcar/status/distance
- patrolcar/events/health

## Suggested final-year extensions

- Replace polling with WebSocket stream.
- Add camera pipeline and object detection trigger.
- Add mission planner that sends autonomous route points.
