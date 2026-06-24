#include <M5Unified.h>
#include <M5_RoverC.h>
#include "config.h"
#include "motion_controller.h"
#include "gripper_controller.h"
#include "action_sequencer.h"
#include "behaviors.h"
#include "remote_handler.h"

M5_ROVERC roverc;
MotionController motion(roverc);
GripperController gripper(roverc);
ActionSequencer sequencer(motion, gripper);
Behaviors behaviors(sequencer);
RemoteHandler remote(motion, gripper, sequencer, behaviors);

OpMode mode = MODE_REMOTE;
unsigned long lastDrawMs = 0;

// Button-mode state
uint8_t btnDirIndex = 0;
const uint8_t BTN_DIR_COUNT = 7;
const char* btnDirNames[] = {"STOP", "FWD", "BCK", "CW", "CCW", "L", "R"};

void applyBtnDirection() {
    switch (btnDirIndex) {
        case 0: motion.stop();                        break;
        case 1: motion.forward(DEFAULT_SPEED);        break;
        case 2: motion.backward(DEFAULT_SPEED);       break;
        case 3: motion.rotateCW(DEFAULT_SPEED);       break;
        case 4: motion.rotateCCW(DEFAULT_SPEED);      break;
        case 5: motion.strafeLeft(DEFAULT_SPEED);     break;
        case 6: motion.strafeRight(DEFAULT_SPEED);    break;
    }
}

void handleButtons() {
    // --- Button A ---
    if (M5.BtnA.wasClicked()) {
        mode = static_cast<OpMode>((mode + 1) % MODE_COUNT);
        Serial.printf("Mode -> %d\n", mode);
        motion.stop();
        sequencer.clear();
        M5.Speaker.tone(800, 50);
    }
    if (M5.BtnA.wasHold()) {
        gripper.toggle();
        Serial.println(gripper.isOpen() ? "Gripper OPEN" : "Gripper CLOSED");
        M5.Speaker.tone(1200, 50);
    }

    // --- Button B ---
    if (M5.BtnB.wasClicked()) {
        switch (mode) {
            case MODE_BUTTON:
                btnDirIndex = (btnDirIndex + 1) % BTN_DIR_COUNT;
                applyBtnDirection();
                M5.Speaker.tone(600, 30);
                break;
            case MODE_DEMO: {
                static uint8_t demoIdx = 0;
                const char* demoNames[] = {"dance", "patrol", "zigzag", "grabDrop"};
                const uint8_t DEMO_COUNT = 4;
                behaviors.start(demoNames[demoIdx]);
                demoIdx = (demoIdx + 1) % DEMO_COUNT;
                M5.Speaker.tone(1000, 50);
                break;
            }
            default: break;
        }
    }
    if (M5.BtnB.wasHold()) {
        motion.stop();
        sequencer.clear();
        btnDirIndex = 0;
        M5.Speaker.tone(400, 100);
    }
}

void drawDisplay() {
    M5.Display.startWrite();
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1);

    // Row 1: mode + battery
    const char* modeNames[] = {"REMOTE", "BUTTON", "DEMO"};
    int bat = M5.Power.getBatteryLevel();
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.printf("%-6s", modeNames[mode]);
    M5.Display.setTextColor(bat > 20 ? TFT_GREEN : TFT_RED);
    M5.Display.printf("BAT:%d%%\n", bat);

    // Row 2: status
    M5.Display.setTextColor(TFT_WHITE);
    if (sequencer.isRunning()) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.println("BEHAVIOR ACTIVE");
    } else if (mode == MODE_BUTTON) {
        M5.Display.print("DIR: ");
        M5.Display.print(btnDirNames[btnDirIndex]);
        M5.Display.print(" ");
        M5.Display.println(gripper.isOpen() ? "OPEN" : "CLOSED");
    } else if (mode == MODE_DEMO) {
        M5.Display.println("BtnB: next demo");
    } else {
        M5.Display.println("BLE: advertising");
    }

    // Row 3: speed
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 48);
    M5.Display.printf("X:%+4d Y:%+4d Z:%+4d\n",
                      motion.currentX(), motion.currentY(), motion.currentZ());

    // Speed bars
    int barY = 64;
    int barW = 80;
    auto drawBar = [&](int y, int8_t val, uint16_t color) {
        M5.Display.fillRect(50, y, barW, 6, TFT_DARKGREY);
        int px = map(abs(val), 0, 100, 0, barW / 2);
        int cx = 50 + barW / 2;
        if (val > 0) M5.Display.fillRect(cx, y, px, 6, color);
        if (val < 0) M5.Display.fillRect(cx - px, y, px, 6, color);
    };
    drawBar(barY, motion.currentY(), TFT_BLUE);
    drawBar(barY + 8, motion.currentX(), TFT_GREEN);
    drawBar(barY + 16, motion.currentZ(), TFT_RED);

    M5.Display.setCursor(50, barY + 24);
    M5.Display.setTextColor(TFT_DARKGREY);
    M5.Display.println("Y   X   Z");

    M5.Display.endWrite();
}

// Demo auto-advance when idle
unsigned long lastDemoAdvanceMs = 0;
void demoAutoAdvance() {
    if (mode != MODE_DEMO) return;
    if (sequencer.isRunning()) return;
    if (millis() - lastDemoAdvanceMs < 3000) return;
    lastDemoAdvanceMs = millis();

    static uint8_t demoIdx = 0;
    const char* demoNames[] = {"dance", "patrol", "zigzag", "grabDrop"};
    const uint8_t DEMO_COUNT = 4;
    behaviors.start(demoNames[demoIdx]);
    demoIdx = (demoIdx + 1) % DEMO_COUNT;
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    Serial.begin(115200);
    Serial.println("DanCar v0.1 booting...");

    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(0, 0);
    M5.Display.println("DanCar v0.1");
    Serial.println("Display init OK");

    if (!roverc.begin(&Wire, 0, 26, 0x38)) {
        Serial.println("RoverC FAIL");
        M5.Display.println("RoverC FAIL");
        M5.Speaker.tone(200, 500);
        while (1) { delay(1000); }
    }
    Serial.println("RoverC OK");
    M5.Display.println("RoverC OK");

    motion.begin();
    sequencer.begin();
    remote.begin();
    Serial.println("BLE advertising as DanCar");

    // Startup chime
    M5.Speaker.tone(880, 80); delay(80);
    M5.Speaker.tone(1100, 80); delay(80);
    M5.Speaker.tone(1320, 120);

    Serial.println("DanCar ready. A:mode B:action Hold-B:STOP");
    M5.Display.println("A:mode  B:action");
    M5.Display.println("Hold B: STOP");
}

void loop() {
    M5.update();  // required for button state machine

    handleButtons();
    motion.update();
    sequencer.tick();
    demoAutoAdvance();

    if (millis() - lastDrawMs > STATUS_REFRESH_MS) {
        lastDrawMs = millis();
        drawDisplay();
    }

    static unsigned long lastBleStatusMs = 0;
    if (millis() - lastBleStatusMs > 2000) {
        lastBleStatusMs = millis();
        remote.notifyStatus();
    }

    delay(5);
}
