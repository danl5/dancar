#include "gripper_controller.h"
#include "config.h"

GripperController::GripperController(M5_ROVERC &roverc)
    : _roverc(roverc), _currentAngle(GRIPPER_OPEN_ANGLE),
      _openAngle(GRIPPER_OPEN_ANGLE), _closeAngle(GRIPPER_CLOSE_ANGLE) {}

void GripperController::open() {
    _currentAngle = _openAngle;
    _roverc.setServoAngle(GRIPPER_SERVO_POS, _currentAngle);
}

void GripperController::close() {
    _currentAngle = _closeAngle;
    _roverc.setServoAngle(GRIPPER_SERVO_POS, _currentAngle);
}

void GripperController::setAngle(uint8_t angle) {
    _currentAngle = angle;
    _roverc.setServoAngle(GRIPPER_SERVO_POS, angle);
}

void GripperController::setServoAngle(uint8_t pos, uint8_t angle) {
    if (pos == GRIPPER_SERVO_POS) _currentAngle = angle;
    _roverc.setServoAngle(pos, angle);
}

void GripperController::toggle() {
    if (isOpen()) {
        close();
    } else {
        open();
    }
}
