#include "action_sequencer.h"
#include "motion_controller.h"
#include "gripper_controller.h"

ActionSequencer::ActionSequencer(MotionController &motion, GripperController &gripper)
    : _motion(motion), _gripper(gripper),
      _head(0), _tail(0), _actionStartMs(0), _delayStartMs(0),
      _running(false), _inDelay(false) {}

void ActionSequencer::begin() {
    _head = _tail = 0;
    _running = false;
    _inDelay = false;
}

void ActionSequencer::tick() {
    if (!_running) return;

    if (_inDelay) {
        if (millis() - _delayStartMs >= _queue[_head].durationMs) {
            _inDelay = false;
            _head = (_head + 1) % QUEUE_SIZE;
            if (_head == _tail) {
                _running = false;
                _motion.stop();
                return;
            }
            executeAction(_queue[_head]);
        }
        return;
    }

    // Check if current action duration has elapsed
    if (_queue[_head].durationMs > 0) {
        if (millis() - _actionStartMs >= _queue[_head].durationMs) {
            finishAction();
            _head = (_head + 1) % QUEUE_SIZE;
            if (_head == _tail) {
                _running = false;
                _motion.stop();
                return;
            }
            executeAction(_queue[_head]);
        }
    }
}

void ActionSequencer::run(ActionCommand cmd) {
    clear();
    enqueue(cmd);
    _running = true;
    _inDelay = false;
    executeAction(_queue[_head]);
}

void ActionSequencer::runSequence(const ActionCommand *cmds, uint8_t count) {
    clear();
    for (uint8_t i = 0; i < count; i++) {
        enqueue(cmds[i]);
    }
    _running = true;
    _inDelay = false;
    executeAction(_queue[_head]);
}

void ActionSequencer::clear() {
    _head = _tail = 0;
    _running = false;
    _inDelay = false;
    _motion.stop();
}

bool ActionSequencer::enqueue(ActionCommand cmd) {
    uint8_t next = (_tail + 1) % QUEUE_SIZE;
    if (next == _head) return false;  // queue full
    _queue[_tail] = cmd;
    _tail = next;
    return true;
}

ActionCommand ActionSequencer::dequeue() {
    ActionCommand cmd = _queue[_head];
    _head = (_head + 1) % QUEUE_SIZE;
    return cmd;
}

void ActionSequencer::executeAction(const ActionCommand &cmd) {
    _actionStartMs = millis();

    switch (cmd.type) {
        case ACTION_MOVE:
            _motion.setRawSpeed(cmd.param1, cmd.param2, cmd.param3);
            break;
        case ACTION_STOP:
            _motion.stop();
            break;
        case ACTION_SERVO:
            _gripper.setServoAngle(cmd.param1, cmd.param2);
            break;
        case ACTION_DELAY:
            _inDelay = true;
            _delayStartMs = millis();
            break;
    }
}

void ActionSequencer::finishAction() {
    // Stop motors after move actions that had a duration
    if (_queue[_head].type == ACTION_MOVE) {
        _motion.stop();
    }
}
