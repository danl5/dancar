#pragma once

#include <M5Unified.h>
#include <M5_RoverC.h>

class MotionController {
public:
    MotionController(M5_ROVERC &roverc);

    void begin();
    void update();

    // Set target speed; ramping happens in update()
    void forward(uint8_t speed);
    void backward(uint8_t speed);
    void strafeLeft(uint8_t speed);
    void strafeRight(uint8_t speed);
    void rotateCW(uint8_t speed);
    void rotateCCW(uint8_t speed);
    void stop();

    void setRawSpeed(int8_t x, int8_t y, int8_t z);

    // Getters
    int8_t currentX() const { return _currentX; }
    int8_t currentY() const { return _currentY; }
    int8_t currentZ() const { return _currentZ; }
    int8_t targetX()  const { return _targetX; }
    int8_t targetY()  const { return _targetY; }
    int8_t targetZ()  const { return _targetZ; }
    bool isMoving()   const { return _motorsActive; }
    unsigned long lastCommandTime() const { return _lastCmdMs; }

private:
    M5_ROVERC &_roverc;

    int8_t _targetX, _targetY, _targetZ;
    int8_t _currentX, _currentY, _currentZ;
    unsigned long _lastRampMs;
    unsigned long _lastCmdMs;
    bool _motorsActive;

    void setTarget(int8_t x, int8_t y, int8_t z);
    static int8_t ramp(int8_t current, int8_t target, int8_t step);
};
