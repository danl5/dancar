#include "remote_handler.h"
#include "motion_controller.h"
#include "gripper_controller.h"
#include "action_sequencer.h"
#include "behaviors.h"
#include "config.h"

RemoteHandler::RemoteHandler(MotionController &motion,
                             GripperController &gripper,
                             ActionSequencer &sequencer,
                             Behaviors &behaviors)
    : _motion(motion), _gripper(gripper), _sequencer(sequencer),
      _behaviors(behaviors), _pServer(nullptr), _pCmdChar(nullptr),
      _pStatusChar(nullptr), _deviceConnected(false) {}

void RemoteHandler::begin() {
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // max BLE TX power

    _pServer = NimBLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks(this));

    NimBLEService *pService = _pServer->createService(BLE_SERVICE_UUID);

    _pCmdChar = pService->createCharacteristic(
        BLE_CHAR_COMMAND,
        NIMBLE_PROPERTY::WRITE
    );
    _pCmdChar->setCallbacks(new CommandCallbacks(this));

    _pStatusChar = pService->createCharacteristic(
        BLE_CHAR_STATUS,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setName(BLE_DEVICE_NAME);
    NimBLEAdvertisementData scanResp;
    scanResp.setName(BLE_DEVICE_NAME);
    pAdvertising->setScanResponseData(scanResp);
    NimBLEDevice::startAdvertising();

    M5.Display.println("BLE: DanCar advertising");
}

void RemoteHandler::update() {
    // NimBLE handles events internally; nothing needed per tick
}

void RemoteHandler::notifyStatus() {
    if (!_deviceConnected) return;

    JsonDocument doc;
    doc["connected"] = true;
    doc["battery"] = M5.Power.getBatteryLevel();
    doc["action"] = _sequencer.isRunning() ? "running" : "idle";

    char buf[JSON_BUF_SIZE];
    serializeJson(doc, buf, sizeof(buf));
    _pStatusChar->setValue(buf);
    _pStatusChar->notify();
}

void RemoteHandler::parseCommand(const char *json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);

    if (err) {
        M5.Display.printf("JSON parse error: %s\n", err.c_str());
        return;
    }

    const char *cmd = doc["cmd"];
    if (!cmd) return;

    JsonObject obj = doc.as<JsonObject>();

    if (strcmp(cmd, "move") == 0) {
        handleMove(obj);
    } else if (strcmp(cmd, "grip") == 0) {
        handleGrip(obj);
    } else if (strcmp(cmd, "stop") == 0) {
        _sequencer.clear();
        _motion.stop();
    } else if (strcmp(cmd, "behavior") == 0) {
        handleBehavior(obj);
    }

    notifyStatus();
}

void RemoteHandler::handleMove(JsonObject obj) {
    const char *dir = obj["dir"] | "forward";
    uint8_t speed = obj["speed"] | DEFAULT_SPEED;
    uint16_t dur = obj["dur"] | 0;

    if (dur > 0) {
        // Timed move via sequencer (auto-stop after duration)
        ActionCommand ac;
        ac.type = ACTION_MOVE;
        ac.durationMs = dur;

        if (strcmp(dir, "forward") == 0)      { ac.param1 =  speed; ac.param2 = 0; ac.param3 = 0; }
        else if (strcmp(dir, "backward") == 0){ ac.param1 = -speed; ac.param2 = 0; ac.param3 = 0; }
        else if (strcmp(dir, "left") == 0)    { ac.param1 = 0; ac.param2 = -speed; ac.param3 = 0; }
        else if (strcmp(dir, "right") == 0)   { ac.param1 = 0; ac.param2 =  speed; ac.param3 = 0; }
        else if (strcmp(dir, "cw") == 0)      { ac.param1 = 0; ac.param2 = 0; ac.param3 =  speed; }
        else if (strcmp(dir, "ccw") == 0)     { ac.param1 = 0; ac.param2 = 0; ac.param3 = -speed; }
        else { ac.param1 = speed; ac.param2 = 0; ac.param3 = 0; }

        _sequencer.clear();
        _sequencer.run(ac);
    } else {
        // Continuous move (until stop or timeout)
        _sequencer.clear();

        if (strcmp(dir, "forward") == 0)       _motion.forward(speed);
        else if (strcmp(dir, "backward") == 0) _motion.backward(speed);
        else if (strcmp(dir, "left") == 0)     _motion.strafeLeft(speed);
        else if (strcmp(dir, "right") == 0)    _motion.strafeRight(speed);
        else if (strcmp(dir, "cw") == 0)       _motion.rotateCW(speed);
        else if (strcmp(dir, "ccw") == 0)      _motion.rotateCCW(speed);
        else                                    _motion.forward(speed);
    }
}

void RemoteHandler::handleGrip(JsonObject obj) {
    const char *action = obj["action"] | "";
    uint8_t pos  = obj["pos"]  | 0;   // 0=gripper, 1/2=arm joints
    uint8_t angle = obj["angle"] | 90;

    if (strcmp(action, "open") == 0) {
        _gripper.setServoAngle(pos, GRIPPER_OPEN_ANGLE);
    } else if (strcmp(action, "close") == 0) {
        _gripper.setServoAngle(pos, GRIPPER_CLOSE_ANGLE);
    } else if (strcmp(action, "toggle") == 0) {
        _gripper.setServoAngle(pos, _gripper.currentAngle() > 30 ? GRIPPER_CLOSE_ANGLE : GRIPPER_OPEN_ANGLE);
    } else {
        _gripper.setServoAngle(pos, angle);
    }
}

void RemoteHandler::handleBehavior(JsonObject obj) {
    const char *name = obj["name"];
    if (!name) return;
    _behaviors.start(name);
}

// --- Server callbacks ---

void RemoteHandler::ServerCallbacks::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) {
    _handler->_deviceConnected = true;
    Serial.println("BLE connected");
}

void RemoteHandler::ServerCallbacks::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) {
    _handler->_deviceConnected = false;
    _handler->_motion.stop();
    Serial.printf("BLE disconnected (reason=%d), re-advertising...\n", reason);
    delay(50);
    NimBLEDevice::startAdvertising();
}

// --- Command callbacks ---

void RemoteHandler::CommandCallbacks::onWrite(NimBLECharacteristic *pChar, NimBLEConnInfo &connInfo) {
    std::string value = pChar->getValue();
    if (!value.empty()) {
        _handler->parseCommand(value.c_str());
    }
}
