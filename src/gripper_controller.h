#pragma once

#include <M5_RoverC.h>

class GripperController {
public:
    GripperController(M5_ROVERC &roverc);

    void open();
    void close();
    void setAngle(uint8_t angle);
    void setServoAngle(uint8_t pos, uint8_t angle);
    void toggle();

    // State
    uint8_t currentAngle() const { return _currentAngle; }
    bool isOpen() const   { return _currentAngle >= _openAngle - 5; }
    bool isClosed() const { return _currentAngle <= _closeAngle + 5; }

private:
    M5_ROVERC &_roverc;
    uint8_t _currentAngle;
    uint8_t _openAngle;
    uint8_t _closeAngle;
};
