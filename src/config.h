#pragma once

// --- BLE ---
#define BLE_SERVICE_UUID    "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_COMMAND    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_CHAR_STATUS     "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define BLE_DEVICE_NAME     "DanCar"

// --- Motor ---
#define MOTOR_SAFETY_TIMEOUT_MS  5000
#define MOTOR_ACCEL_STEP          5       // speed units per ramp tick
#define MOTOR_ACCEL_INTERVAL_MS   20      // ms between ramp steps
#define DEFAULT_SPEED             80

// --- Servo (gripper) ---
#define GRIPPER_CLOSE_ANGLE   60
#define GRIPPER_OPEN_ANGLE    10
#define GRIPPER_SERVO_POS     0
#define GRIPPER_MOVE_DUR_MS   600     // ms for gripper servo movement

// --- UI ---
#define BUTTON_DEBOUNCE_MS        50
#define BUTTON_LONG_PRESS_MS      800
#define STATUS_REFRESH_MS         200
#define DISPLAY_WIDTH             135
#define DISPLAY_HEIGHT            240

// --- Audio ---
#define AUDIO_DEFAULT_VOLUME 15

// --- Operating modes ---
enum OpMode {
    MODE_REMOTE,    // controlled via BLE
    MODE_BUTTON,    // controlled via onboard buttons
    MODE_DEMO,      // autonomous demo sequence
    MODE_COUNT
};

// --- Action types for the sequencer ---
enum ActionType {
    ACTION_MOVE,
    ACTION_STOP,
    ACTION_SERVO,
    ACTION_DELAY,
    ACTION_AUDIO,
};
