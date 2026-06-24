#include "motion_controller.h"
#include "config.h"

MotionController::MotionController(M5_ROVERC &roverc)
    : _roverc(roverc), _targetX(0), _targetY(0), _targetZ(0),
      _currentX(0), _currentY(0), _currentZ(0),
      _lastRampMs(0), _lastCmdMs(0), _motorsActive(false) {}

void MotionController::begin() {
    _roverc.setSpeed(0, 0, 0);
    _targetX = _targetY = _targetZ = 0;
    _currentX = _currentY = _currentZ = 0;
}

void MotionController::update() {
    // Safety timeout
    if (_motorsActive && (millis() - _lastCmdMs > MOTOR_SAFETY_TIMEOUT_MS)) {
        setTarget(0, 0, 0);
    }

    // Speed ramping — interpolate towards target
    if (millis() - _lastRampMs < MOTOR_ACCEL_INTERVAL_MS) return;
    _lastRampMs = millis();

    int8_t cx = ramp(_currentX, _targetX, MOTOR_ACCEL_STEP);
    int8_t cy = ramp(_currentY, _targetY, MOTOR_ACCEL_STEP);
    int8_t cz = ramp(_currentZ, _targetZ, MOTOR_ACCEL_STEP);

    if (cx != _currentX || cy != _currentY || cz != _currentZ) {
        _currentX = cx;
        _currentY = cy;
        _currentZ = cz;
        _roverc.setSpeed(cy, cx, cz);  // x/y swaped for physical wheel orientation

        if (cx == 0 && cy == 0 && cz == 0) {
            _motorsActive = false;
        }
    }
}

void MotionController::forward(uint8_t speed)       { setTarget(speed, 0, 0); }
void MotionController::backward(uint8_t speed)      { setTarget(-(int8_t)speed, 0, 0); }
void MotionController::strafeLeft(uint8_t speed)    { setTarget(0, -(int8_t)speed, 0); }
void MotionController::strafeRight(uint8_t speed)   { setTarget(0, speed, 0); }
void MotionController::rotateCW(uint8_t speed)      { setTarget(0, 0, speed); }
void MotionController::rotateCCW(uint8_t speed)     { setTarget(0, 0, -(int8_t)speed); }

void MotionController::stop() {
    setTarget(0, 0, 0);
}

void MotionController::setRawSpeed(int8_t x, int8_t y, int8_t z) {
    _currentX = _targetX = x;
    _currentY = _targetY = y;
    _currentZ = _targetZ = z;
    _roverc.setSpeed(y, x, z);  // x/y swaped for physical wheel orientation
    _motorsActive = (x != 0 || y != 0 || z != 0);
    _lastCmdMs = millis();
}

void MotionController::setTarget(int8_t x, int8_t y, int8_t z) {
    if (x != _targetX || y != _targetY || z != _targetZ) {
        _lastCmdMs = millis();
    }
    _targetX = x;
    _targetY = y;
    _targetZ = z;
    if (x != 0 || y != 0 || z != 0) {
        _motorsActive = true;
    }
}

int8_t MotionController::ramp(int8_t current, int8_t target, int8_t step) {
    if (current < target) {
        return (current + step < target) ? current + step : target;
    } else if (current > target) {
        return (current - step > target) ? current - step : target;
    }
    return current;
}
