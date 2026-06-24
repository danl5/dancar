#pragma once

#include <Arduino.h>
#include "config.h"

struct ActionCommand {
    ActionType type;
    int8_t param1;  // e.g., x / servo position
    int8_t param2;  // e.g., y / servo angle
    int8_t param3;  // e.g., z
    uint16_t durationMs;  // how long to sustain (0 = instantaneous)
};

class ActionSequencer {
public:
    ActionSequencer(class MotionController &motion, class GripperController &gripper);

    void begin();
    void tick();           // call each loop iteration

    // Queue one action (blocking for durationMs if > 0)
    void run(ActionCommand cmd);

    // Queue a sequence of actions
    void runSequence(const ActionCommand *cmds, uint8_t count);

    // Clear pending queue
    void clear();

    bool isRunning() const { return _running; }

private:
    MotionController &_motion;
    GripperController &_gripper;

    static const uint8_t QUEUE_SIZE = 32;
    ActionCommand _queue[QUEUE_SIZE];
    uint8_t _head;
    uint8_t _tail;

    unsigned long _actionStartMs;
    unsigned long _delayStartMs;
    bool _running;
    bool _inDelay;

    bool enqueue(ActionCommand cmd);
    ActionCommand dequeue();
    void executeAction(const ActionCommand &cmd);
    void finishAction();
};
