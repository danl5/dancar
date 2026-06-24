#include "behaviors.h"
#include "config.h"

// --- Built-in behavior definitions ---

const ActionCommand Behaviors::DANCE[] = {
    { ACTION_MOVE,  50, 0, 0,   400 },
    { ACTION_MOVE, -50, 0, 0,   400 },
    { ACTION_MOVE,   0, 0, 60,  500 },
    { ACTION_MOVE,   0, 0, -60, 500 },
    { ACTION_MOVE,  50, 20, 0,  300 },
    { ACTION_MOVE, -50, -20, 0, 300 },
    { ACTION_STOP,   0, 0, 0,     0 },
};
const uint8_t Behaviors::DANCE_COUNT = sizeof(DANCE) / sizeof(ActionCommand);

const ActionCommand Behaviors::PATROL[] = {
    { ACTION_MOVE,  60, 0, 0, 1500 },
    { ACTION_MOVE,   0, 0, 50,  800 },
    { ACTION_MOVE,  60, 0, 0, 1000 },
    { ACTION_MOVE,   0, 0, -60, 600 },
    { ACTION_MOVE, -60, 0, 0, 1500 },
    { ACTION_MOVE,   0, 0, 60,  800 },
    { ACTION_STOP,   0, 0, 0,    0 },
};
const uint8_t Behaviors::PATROL_COUNT = sizeof(PATROL) / sizeof(ActionCommand);

const ActionCommand Behaviors::GRAB_DROP[] = {
    { ACTION_SERVO, 0, 60, 0, 500 },
    { ACTION_MOVE,  40, 0, 0, 2000 },
    { ACTION_STOP,   0, 0, 0,  500 },
    { ACTION_SERVO, 0, 10, 0, 500 },
    { ACTION_MOVE, -40, 0, 0, 2000 },
    { ACTION_SERVO, 0, 60, 0, 500 },
    { ACTION_STOP,   0, 0, 0,    0 },
};
const uint8_t Behaviors::GRAB_DROP_COUNT = sizeof(GRAB_DROP) / sizeof(ActionCommand);

const ActionCommand Behaviors::ZIGZAG[] = {
    { ACTION_MOVE, 60, 30,  0, 500 },
    { ACTION_MOVE, 60, -30, 0, 500 },
    { ACTION_MOVE, 60, 30,  0, 500 },
    { ACTION_MOVE, 60, -30, 0, 500 },
    { ACTION_MOVE, 60, 30,  0, 500 },
    { ACTION_STOP,  0,  0,  0,   0 },
};
const uint8_t Behaviors::ZIGZAG_COUNT = sizeof(ZIGZAG) / sizeof(ActionCommand);

// --- Behaviors class implementation ---

Behaviors::Behaviors(ActionSequencer &sequencer)
    : _sequencer(sequencer), _registryCount(0) {
    // Register built-in behaviors
    add("dance",    DANCE,    DANCE_COUNT);
    add("patrol",   PATROL,   PATROL_COUNT);
    add("grabDrop", GRAB_DROP, GRAB_DROP_COUNT);
    add("zigzag",   ZIGZAG,   ZIGZAG_COUNT);
}

bool Behaviors::start(const char *name) {
    for (uint8_t i = 0; i < _registryCount; i++) {
        if (strcmp(_registry[i].name, name) == 0) {
            _sequencer.runSequence(_registry[i].cmds, _registry[i].count);
            return true;
        }
    }
    return false;
}

void Behaviors::add(const char *name, const ActionCommand *cmds, uint8_t count) {
    if (_registryCount >= MAX_BEHAVIORS) return;
    _registry[_registryCount].name = name;
    _registry[_registryCount].cmds = cmds;
    _registry[_registryCount].count = count;
    _registryCount++;
}

bool Behaviors::isRunning() const {
    return _sequencer.isRunning();
}
