# DanCar

M5StickC Plus + RoverC Pro robot with gripper arm. Three control modes: BLE remote, onboard buttons, autonomous demo.

## Hardware

- **Controller**: M5StickC Plus (ESP32-PICO-D4, 240MHz, 4MB Flash)
- **Chassis**: RoverC Pro — 4× N20 worm-gear motors + Mecanum wheels, 1× gripper servo + 2× expansion servo ports, STM32F030 I2C coprocessor @ 0x38
- **Connection**: StickC Plus docks directly onto RoverC Pro HAT

## Features

- **BLE Remote**: Phone/PC sends JSON commands over BLE GATT to control movement, gripper, and trigger preset behaviors
- **Button Mode**: Onboard A/B buttons for direction switching and gripper control
- **Demo Mode**: 4 preset behaviors auto-cycle (dance, patrol, zigzag, grabDrop)
- **Display**: Live status (mode / battery / direction / speed)

## Build & Flash

```bash
# Compile
platformio run

# Flash and monitor
platformio run --target upload && platformio device monitor
```

Dependencies managed automatically via `platformio.ini`: M5Unified, M5GFX, M5_RoverC, NimBLE-Arduino, ArduinoJson.

## BLE Control

Connect to device named `DanCar`.

| Command | JSON |
|---------|------|
| Move | `{"cmd":"move","dir":"forward","speed":80,"dur":500}` |
| Gripper | `{"cmd":"grip","action":"close"}` |
| Stop | `{"cmd":"stop"}` |
| Behavior | `{"cmd":"behavior","name":"dance"}` |

Directions: `forward / backward / left / right / cw / ccw`. Gripper: `open / close / angle:N`. Behaviors: `dance / patrol / zigzag / grabDrop`.

Test with nRF Connect (Android/iOS) or LightBlue (macOS).

## Button Controls

| Button | Short Press | Long Press |
|--------|-------------|------------|
| A | Cycle mode (Remote → Button → Demo) | Toggle gripper open/close |
| B | Mode action (advance direction / next demo) | Emergency stop + reset |
