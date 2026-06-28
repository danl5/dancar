# DanCar

M5StickC Plus + RoverC Pro robot with gripper arm. Three control modes: BLE remote, onboard buttons, autonomous demo.

## Hardware

- **Controller**: M5StickC Plus (ESP32-PICO-D4, 240MHz, 4MB Flash)
- **Chassis**: RoverC Pro — 4× N20 worm-gear motors + Mecanum wheels, 1× gripper servo + 2× expansion servo ports, STM32F030 I2C coprocessor @ 0x38
- **Connection**: StickC Plus docks directly onto RoverC Pro HAT

## Build & Flash

```bash
platformio run
platformio run --target upload && platformio device monitor
```

Dependencies managed via `platformio.ini`: M5Unified, M5GFX, M5_RoverC, NimBLE-Arduino, ArduinoJson, M5Unit-AudioPlayer.

---

## Interfaces

### BLE GATT

Device name: `DanCar`. Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`.

#### Command (write, `beb5483e-36e1-4688-b7f5-ea07361b26a8`)

JSON payloads:

**`move`** — drive in a direction.

| Param | Type | Values | Default |
|-------|------|--------|---------|
| `dir` | string | `forward`, `backward`, `left`, `right`, `cw`, `ccw` | `forward` |
| `speed` | uint8 | 0–100 | 80 |
| `dur` | uint16 | duration in ms (timed move); omit or `0` for continuous | `0` |

Examples:
```json
{"cmd":"move","dir":"forward","speed":60}
{"cmd":"move","dir":"left","speed":80,"dur":500}
{"cmd":"move","dir":"cw","speed":30,"dur":1000}
```

**`grip`** — control a servo (gripper or arm joint).

| Param | Type | Values | Default |
|-------|------|--------|---------|
| `action` | string | `open`, `close`, `toggle` | — |
| `pos` | uint8 | `0` (gripper), `1` (arm 1), `2` (arm 2) | `0` |
| `angle` | uint8 | 0–180 (used when action omitted) | 90 |

Examples:
```json
{"cmd":"grip","action":"close"}
{"cmd":"grip","action":"toggle"}
{"cmd":"grip","pos":0,"angle":45}
{"cmd":"grip","action":"open","pos":1}
```

**`stop`** — halt all motors and clear action queue.

```json
{"cmd":"stop"}
```

**`behavior`** — trigger a preset action sequence.

| Param | Type | Values |
|-------|------|--------|
| `name` | string | `dance`, `patrol`, `grabDrop`, `zigzag` |

```json
{"cmd":"behavior","name":"dance"}
```

**`audio`** — control the AudioPlayer unit (N9301, Grove port).

| Param | Type | Values | Default |
|-------|------|--------|---------|
| `action` | string | `play`, `stop`, `pause`, `resume`, `next`, `prev`, `volume` | — |
| `name` | string | track name (see below) | — |
| `index` | uint8 | 1–8, file number on SD card | — |
| `vol` | uint8 | 0–30 | 15 |

Track names:

| Name | File |
|------|------|
| `yahaha` | Korok discovery sound |
| `sensor` | Sheikah Sensor ping |
| `double` | Double Sensor ping |
| `item` | Item collect |
| `sword` | Master Sword get |
| `guardian` | Guardian laser |
| `rocks` | Rocks drop |
| `contact` | First contact |

Examples:
```json
{"cmd":"audio","action":"play","name":"yahaha"}
{"cmd":"audio","action":"play","index":1}
{"cmd":"audio","action":"stop"}
{"cmd":"audio","action":"next"}
{"cmd":"audio","action":"volume","vol":20}
```

#### Status (read / notify, `beb5483e-36e1-4688-b7f5-ea07361b26a9`)

JSON payload, pushed every 2s when connected:

| Field | Type | Description |
|-------|------|-------------|
| `connected` | bool | `true` when BLE link active |
| `battery` | uint8 | Battery level 0–100% |
| `action` | string | `"idle"` or `"running"` |

Example: `{"connected":true,"battery":78,"action":"idle"}`

Test with nRF Connect (Android/iOS) or LightBlue (macOS).

### Web Bluetooth (`controller.html`)

Single-file browser UI using Web Bluetooth API (Chrome/Edge only).

| Control | Input | Sends |
|---------|-------|-------|
| D-pad | pointer / touch | `move` continuous (release → `stop`) |
| Rotate | pointer drag | `move` dir=`cw`/`ccw` |
| Speed slider | 0–100 | embeds in every `move` command |
| Gripper button | click | `grip` action=`toggle` |
| Behavior buttons (1–4) | click | `behavior` name=`dance`/`patrol`/`grabDrop`/`zigzag` |

Keyboard shortcuts: `W`=forward, `S`=backward, `A`=left, `D`=right, `Q`=rotate CCW, `E`=rotate CW, `Space`=stop, `1`–`4`=behaviors.

### Onboard buttons

| Button | Short press | Long press |
|--------|-------------|------------|
| **A** | Cycle mode: Remote → Button → Demo | Toggle gripper open/close |
| **B** | Mode-specific (see below) | Emergency stop + reset to idle |

**Button Mode** (mode 1): BtnB cycles 7 directions — STOP → FWD → BCK → CW → CCW → LEFT → RIGHT.

**Demo Mode** (mode 2): BtnB advances to next behavior. Auto-cycles every 3s when idle.

### Display

| Row | Content |
|-----|---------|
| 1 | Mode name + battery % |
| 2 | Status: BEHAVIOR ACTIVE / DIR: X / BLE: advertising |
| 3 | X/Y/Z speed values (±100) |
| 4–6 | Speed bars (blue=Y, green=X, red=Z) |

### Behaviors

| Name | Description |
|------|-------------|
| `dance` | Forward-back wiggle + spin |
| `patrol` | Forward, rotate, forward, rotate back, return |
| `grabDrop` | Open gripper → drive forward → close → reverse → open |
| `zigzag` | Forward with alternating strafe (5 legs) |
