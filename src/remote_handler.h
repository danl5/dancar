#pragma once

#include <NimBLEDevice.h>
#include <ArduinoJson.h>

class RemoteHandler {
public:
    RemoteHandler(class MotionController &motion,
                  class GripperController &gripper,
                  class ActionSequencer &sequencer,
                  class Behaviors &behaviors);

    void begin();
    void update();  // call each loop iteration

    void notifyStatus();

private:
    MotionController &_motion;
    GripperController &_gripper;
    ActionSequencer &_sequencer;
    Behaviors &_behaviors;

    NimBLEServer *_pServer;
    NimBLECharacteristic *_pCmdChar;
    NimBLECharacteristic *_pStatusChar;
    bool _deviceConnected;

    void parseCommand(const char *json);
    void handleMove(JsonObject obj);
    void handleGrip(JsonObject obj);
    void handleBehavior(JsonObject obj);

    static const uint16_t JSON_BUF_SIZE = 256;

    // BLE callbacks
    class ServerCallbacks : public NimBLEServerCallbacks {
        RemoteHandler *_handler;
    public:
        ServerCallbacks(RemoteHandler *handler) : _handler(handler) {}
        void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override;
        void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override;
    };

    class CommandCallbacks : public NimBLECharacteristicCallbacks {
        RemoteHandler *_handler;
    public:
        CommandCallbacks(RemoteHandler *handler) : _handler(handler) {}
        void onWrite(NimBLECharacteristic *pChar, NimBLEConnInfo &connInfo) override;
    };
};
