#pragma once

#include <Arduino.h>
#include "action_sequencer.h"

class Behaviors {
public:
    Behaviors(ActionSequencer &sequencer);

    // Kick off a named behavior (returns immediately, runs async via sequencer)
    bool start(const char *name);

    // Register a custom behavior (call before start)
    void add(const char *name, const ActionCommand *cmds, uint8_t count);

    bool isRunning() const;

    // --- Built-in behaviors ---
    static const ActionCommand DANCE[];
    static const uint8_t DANCE_COUNT;

    static const ActionCommand PATROL[];
    static const uint8_t PATROL_COUNT;

    static const ActionCommand GRAB_DROP[];
    static const uint8_t GRAB_DROP_COUNT;

    static const ActionCommand ZIGZAG[];
    static const uint8_t ZIGZAG_COUNT;

private:
    ActionSequencer &_sequencer;

    struct NamedBehavior {
        const char *name;
        const ActionCommand *cmds;
        uint8_t count;
    };

    static const uint8_t MAX_BEHAVIORS = 16;
    NamedBehavior _registry[MAX_BEHAVIORS];
    uint8_t _registryCount;
};
