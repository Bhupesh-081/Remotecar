# Sensor Stack

This folder contains reusable sensor assets that turn the patrol car into a measurable robotics platform.

## Included sensor layers

- Ultrasonic ranging (HC-SR04 or equivalent)
- Battery telemetry channel (ADC with voltage divider)
- Optional IMU and wheel encoder placeholders
- Lightweight filtering templates for robust field readings

## Hardware quality checklist

- Use twisted pairs for motor supply and keep sensor ground common.
- Add a 100 nF ceramic capacitor close to each sensor power input.
- Keep ultrasonic trigger and echo wires shorter than 30 cm.
- Place battery divider resistors near the ESP32 ADC pin.

## Suggested expansion for final-year outcomes

- Add IMU fusion with complementary or Kalman filter.
- Add wheel encoder odometry and drift correction.
- Build map-aware patrol metrics and run-time fault scoring.
