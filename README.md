# DanCar

M5StickC Plus + RoverC Pro 夹臂小车, 支持 BLE 遥控、按键控制、自主演示三种模式。

## 硬件

- **主控**: M5StickC Plus (ESP32-PICO-D4, 240MHz, 4MB Flash)
- **底盘**: RoverC Pro — 4×N20 蜗杆电机 + 麦克纳姆轮, 1×夹爪舵机 + 2×扩展舵机口, STM32F030 I2C 协处理器 @ 0x38
- **连接**: StickC Plus 直接插 RoverC Pro HAT

## 功能

- **BLE 遥控**: 手机/电脑通过 BLE GATT 发送 JSON 指令控制移动、夹爪、触发预设动作
- **按键模式**: 板载 A/B 键切换方向和夹爪
- **演示模式**: 4 套预设动作自动轮播 (dance, patrol, zigzag, grabDrop)
- **屏幕显示**: 实时状态 (模式/电量/方向/速度)

## 构建 & 烧录

```bash
# 编译
platformio run

# 烧录 + 串口监视
platformio run --target upload && platformio device monitor
```

依赖通过 `platformio.ini` 自动管理: M5Unified, M5GFX, M5_RoverC, NimBLE-Arduino, ArduinoJson。

## BLE 控制

连接设备名 `DanCar`。

| 指令 | JSON |
|------|------|
| 移动 | `{"cmd":"move","dir":"forward","speed":80,"dur":500}` |
| 夹爪 | `{"cmd":"grip","action":"close"}` |
| 停止 | `{"cmd":"stop"}` |
| 预设动作 | `{"cmd":"behavior","name":"dance"}` |

方向: `forward / backward / left / right / cw / ccw`。夹爪: `open / close / angle:N`。预设: `dance / patrol / zigzag / grabDrop`。

用 nRF Connect (Android/iOS) 或 LightBlue (macOS) 测试。

## 按键操作

| 按键 | 短按 | 长按 |
|------|------|------|
| A | 切换模式 (遥控/按键/演示) | 夹爪开/关 |
| B | 模式内操作 | 急停 + 复位 |
