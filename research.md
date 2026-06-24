# DanCar — Design Document

> M5StickC Plus + RoverC Pro mobile robot with gripper arm.
> Remote-controlled via BLE with preset action sequences.

---

## 1. Technology Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| Build system | **PlatformIO** | Dependency management, VS Code / CLion support, better than Arduino IDE for multi-file projects |
| Framework | **Arduino** (on ESP32) | M5Stack ecosystem is Arduino-first; all official libraries target Arduino |
| Core library | **M5Unified** v0.2.17 + **M5GFX** | Replaces deprecated M5StickCPlus v0.1.1; unified API across M5 devices |
| Chassis driver | **M5_RoverC** v0.0.1 (MIT) | Official I2C driver for RoverC Pro; stable, simple, MIT licensed |
| BLE stack | **NimBLE-Arduino** | Lighter than Bluedroid (~100KB flash saved), better for constrained ESP32 |
| Command parsing | **ArduinoJson** v7 | Parse structured BLE commands (JSON); widely used in M5Stack projects |
| Display | **M5GFX** (included with M5Unified) | High-performance ST7789v2 driver, sprite support |

### 1.1 Why not...

| Rejected | Reason |
|----------|--------|
| ESP-IDF bare | Steep learning curve; M5_RoverC is Arduino-native; no ecosystem value for this project |
| MicroPython / UIFlow2 | Real-time constraints for motor control; complex action sequences outgrow block programming |
| Bluedroid (default BLE) | ~300KB flash overhead; NimBLE achieves same functionality at ~100KB |
| M5StickCPlus (deprecated) | Official deprecation; no future updates; M5Unified is the forward path |
| ESP-NOW remote | Chose BLE for phone/PC compatibility; ESP-NOW can be added later for JoyC |

**Note:** The PlatformIO registry version of M5_RoverC (v0.0.1) uses the class name `M5_ROVERC` (all caps), while the GitHub source uses `M5_RoverC` (mixed case). Our code uses `M5_ROVERC` to match the installed library.

---

## 2. Hardware Reference

### 2.1 M5StickC Plus (SKU: K016-P)

| Parameter | Value |
|-----------|-------|
| SoC | ESP32-PICO-D4 (dual-core, 240MHz) |
| Flash / SRAM | 4MB / 520KB |
| Wireless | WiFi 2.4GHz b/g/n + BLE 4.2 |
| Display | 1.14" TFT, ST7789v2, 135×240 |
| IMU | MPU6886 (0x68) or SH200Q (0x6C, older batch) |
| RTC | BM8563 (0x51) |
| PMU | AXP192 (0x34) |
| Battery | 120mAh @ 3.7V (built-in) |
| Dimensions | 48×24×13.5mm |

**Pin assignments (relevant to RoverC):**

| Function | GPIO | Notes |
|----------|------|-------|
| HAT SDA | G0 | Shared with Mic CLK — mic unavailable when RoverC connected |
| HAT SCL | G26 | |
| Internal I2C SDA | G21 | IMU + PMU + RTC (separate from RoverC I2C pins but same Wire bus) |
| Internal I2C SCL | G22 | |
| Button A | G37 | |
| Button B | G39 | |
| Buzzer | G2 | Passive buzzer |
| LED | G10 | Red LED |
| IR TX | G9 | IR transmitter |

**HAT pinout (bottom connector):**
```
GND | 5V | G26(SCL) | G25/36 | G0(SDA) | 3.3V | 5V
```

**Key constraints:**
- G0 is shared between RoverC I2C (SDA) and microphone CLK — mic is unavailable when RoverC is docked.
- G25 and G36 share the same IO pad — when using G36 as ADC input, G25 must be set to FLOATING.

### 2.2 RoverC Pro (SKU: K036-B)

| Parameter | Value |
|-----------|-------|
| Coprocessor | STM32F030C8T6 |
| I2C slave address | **0x38** |
| Motors | 4× N20 worm-gear motors (L9110S driver) |
| Wheel type | Mecanum (omnidirectional) |
| Gripper | 1× servo (internal, pos=0) |
| Expansion servo ports | 2× (pos=1, pos=2) |
| Expansion Grove | 2× I2C ports |
| Battery | 16340 Li-ion, 700mAh (removable) |
| Dimensions | 120×75×58mm |
| Connection | Direct dock to M5StickC Plus HAT |

**I2C Register Map (from M5_RoverC.cpp source):**

| Register | Function | Range |
|----------|----------|-------|
| 0x00 | Motor 0 speed (int8) | -100 ~ 100 |
| 0x01 | Motor 1 speed (int8) | -100 ~ 100 |
| 0x02 | Motor 2 speed (int8) | -100 ~ 100 |
| 0x03 | Motor 3 speed (int8) | -100 ~ 100 |
| 0x10 | Servo 0 angle (gripper) | 0 ~ 180 |
| 0x11 | Servo 1 angle (expansion) | 0 ~ 180 |
| 0x12 | Servo 2 angle (expansion) | 0 ~ 180 |
| 0x20 | Servo 0 pulse width | uint16 |
| 0x21 | Servo 1 pulse width | uint16 |
| 0x22 | Servo 2 pulse width | uint16 |

**Mecanum kinematics (from `M5_RoverC::setSpeed`):**
```
x: forward/backward (positive = forward)
y: strafe left/right (positive = right)
z: rotate (positive = clockwise)

Motor mixing formula:
  M0 = clamp(y + x - z, -100, 100)
  M1 = clamp(y - x + z, -100, 100)
  M2 = clamp(y - x - z, -100, 100)
  M3 = clamp(y + x + z, -100, 100)

When z != 0, x and y are attenuated proportionally:
  x = x * (100 - |z|) / 100
  y = y * (100 - |z|) / 100
```

---

## 3. Software Architecture

### 3.1 Layered Command Architecture

```
┌─────────────────────────────────────────────────────┐
│  Behavior Layer (behaviors.h)                        │
│  Preset sequences: dance, patrol, grab-and-drop, ... │
├─────────────────────────────────────────────────────┤
│  Action Sequencer (action_sequencer.h)               │
│  Non-blocking command queue with duration/delay       │
├─────────────────────────────────────────────────────┤
│  Motion Primitives (motion_controller.h)              │
│  move(dir, speed, duration), grip(angle), stop()     │
├─────────────────────────────────────────────────────┤
│  Driver Layer                                        │
│  M5_RoverC (I2C) + M5Unified (display, IMU, buttons) │
└─────────────────────────────────────────────────────┘
```

**Rationale:** Separating motion primitives from action sequencing from behaviors allows:
- Remote control to directly call motion primitives
- Preset behaviors to be expressed as reusable sequences
- New behaviors added without touching lower layers
- Each layer testable in isolation

### 3.2 Module Responsibilities

**MotionController** (`motion_controller.h`)
- Wraps `M5_RoverC::setSpeed` and `setServoAngle` with named operations
- Normalizes direction/speed from user input (-100..100 range)
- Auto-stop safety: zeroes motors after timeout if no new command received
- API: `forward(speed)`, `backward(speed)`, `strafeLeft(speed)`, `strafeRight(speed)`, `rotateCW(speed)`, `rotateCCW(speed)`, `stop()`

**GripperController** (`gripper_controller.h`)
- Wraps servo angle control
- Predefined positions: `open()`, `close()`, `setAngle(deg)`
- Optional: multi-servo coordinated sequences (e.g., arm lift + grip)

**ActionSequencer** (`action_sequencer.h`)
- Non-blocking command queue (avoids `delay()` which blocks BLE)
- Each command has: { action_type, params, duration_ms }
- Called from `loop()` — dequeues and executes when previous action duration elapses
- Supports: MOTION, SERVO, DELAY, WAIT_FOR_BUTTON

**BehaviorLibrary** (`behaviors.h`)
- Pre-built action sequences stored as arrays of ActionCommands
- Examples: `dance()`, `patrol()`, `grabAndDrop()`, `zigzag()`, `spinWave()`

**RemoteHandler** (`remote_handler.h`)
- NimBLE GATT server with a custom service
- Single characteristic for bidirectional command/status exchange
- Parses incoming JSON commands, dispatches to MotionController or ActionSequencer
- Reports battery level, IMU data (optional) via notifications

### 3.3 BLE GATT Service Design

```
Service UUID: 4fafc201-1fb5-459e-8fcc-c5c9c331914b

Characteristics:
  Command (RX, write)
    UUID: beb5483e-36e1-4688-b7f5-ea07361b26a8
    Format: JSON string, e.g.:
      {"cmd":"move",  "dir":"forward", "speed":80, "dur":500}
      {"cmd":"grip",  "action":"close"}
      {"cmd":"grip",  "action":"open"}
      {"cmd":"stop"}
      {"cmd":"behavior", "name":"dance"}
      {"cmd":"behavior", "name":"patrol"}

  Status (TX, notify)
    UUID: beb5483e-36e1-4688-b7f5-ea07361b26a9
    Format: JSON string, e.g.:
      {"battery":78, "connected":true, "action":"idle"}
      {"battery":75, "connected":true, "action":"running", "behavior":"dance"}
```

**Command types:**
| cmd | params | Description |
|-----|--------|-------------|
| `move` | dir, speed, dur? | Continuous or timed movement. dir: forward/backward/left/right/cw/ccw |
| `grip` | action | open / close / angle:{deg} |
| `stop` | — | Halt all motors |
| `behavior` | name | Trigger preset: dance, patrol, grabDrop, zigzag, spinWave |

### 3.4 FreeRTOS Task Design

Two tasks for concurrency:
- **Main loop** (core 0): BLE event processing + command dispatch + action sequencer tick
- **Safety watchdog** (core 1, optional): Motor auto-stop if no BLE connection or command timeout (5s)

If the safety task is omitted (single-core), the auto-stop logic runs in the main loop via a `lastCommandMillis` check.

---

## 4. Project Structure

```
dancar/
├── platformio.ini                # Build config, lib deps
├── README.md                     # Project overview (English)
├── src/
│   ├── main.cpp                  # Entry point: M5.begin(), roverc.begin(), BLE init, loop()
│   ├── motion_controller.h       # Motion primitives (forward, strafe, rotate, stop)
│   ├── motion_controller.cpp
│   ├── gripper_controller.h      # Gripper servo control
│   ├── gripper_controller.cpp
│   ├── action_sequencer.h        # Non-blocking action queue
│   ├── action_sequencer.cpp
│   ├── behaviors.h               # Preset behavior sequences
│   ├── behaviors.cpp
│   ├── remote_handler.h          # BLE GATT service + command parser
│   ├── remote_handler.cpp
│   └── config.h                  # Pin definitions, UUIDs, timing constants
└── lib/
    └── M5_RoverC/                # Vendored copy (or PlatformIO lib_deps)
```

### 4.1 `platformio.ini`

```ini
[env:m5stick-c-plus]
platform = espressif32
board = m5stick-c-plus
framework = arduino
monitor_speed = 115200

lib_deps =
    m5stack/M5Unified @ ^0.2.17
    m5stack/M5GFX @ ^0.2.5
    m5stack/M5_RoverC @ ^0.0.1
    h2zero/NimBLE-Arduino @ ^2.2.0
    bblanchon/ArduinoJson @ ^7.0.0
```

---

## 5. Known Constraints & Mitigations

| Constraint | Mitigation |
|------------|------------|
| G0 shared with mic — mic unavailable when RoverC docked | Accept limitation; mic not needed for car operation |
| M5StickC 120mAh battery is small | RoverC's 700mAh battery powers motors via I2C power pins; M5StickC draws from same source when docked |
| M5_RoverC API is blocking (I2C) — no async | Calls are fast (<1ms I2C transaction); fine for loop-based scheduling |
| `delay()` in examples blocks BLE | All our code uses non-blocking `millis()`-based timing via ActionSequencer |
| ESP32-PICO 4MB flash — tight for BLE + display + app | NimBLE saves ~100KB vs Bluedroid; JSON parser memory pool capped at 2KB |
| Mecanum wheels slip on some surfaces | Speed ramping (acceleration profile) in MotionController; not critical for first version |

---

## 6. Development Workflow

1. `platformio run` — compile
2. `platformio run --target upload` — flash via USB-C
3. `platformio device monitor` — serial monitor (115200 baud)
4. BLE testing: nRF Connect (Android/iOS) or LightBlue (macOS) to send JSON commands

---

## 7. References

- [M5StickC Plus docs](https://docs.m5stack.com/en/core/m5stickc_plus)
- [RoverC Pro docs](https://docs.m5stack.com/en/hat/hat_roverc_pro)
- [M5Unified GitHub](https://github.com/m5stack/M5Unified)
- [M5_RoverC GitHub](https://github.com/m5stack/M5-RoverC)
- [NimBLE-Arduino docs](https://github.com/h2zero/NimBLE-Arduino)
- [Official RunningRoverC example](https://github.com/m5stack/M5-RoverC/tree/master/examples/RoverC_M5StickCPlus/RunningRoverC)
