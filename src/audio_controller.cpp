#include "audio_controller.h"
#include "config.h"
#include <M5Unified.h>

const AudioTrack AudioController::TRACKS[] = {
    {"yahaha",    "yahaha.wav"},
    {"sensor",    "sensor.wav"},
    {"double",    "double.wav"},
    {"item",      "item.wav"},
    {"sword",     "sword.wav"},
    {"guardian",  "guardian.wav"},
    {"rocks",     "rocks.wav"},
    {"contact",   "contact.wav"},
};

AudioController::AudioController() : _ready(false) {}

bool AudioController::begin() {
    int8_t rx = M5.getPin(m5::pin_name_t::port_a_pin1);
    int8_t tx = M5.getPin(m5::pin_name_t::port_a_pin2);

    if (!_player.begin(&Serial1, rx, tx)) {
        Serial.println("AudioPlayer begin failed (SD card?)");
        return false;
    }

    _ready = true;
    _player.setVolume(20);
    delay(200);
    _player.setPlayMode(AUDIO_PLAYER_MODE_SINGLE_STOP);
    delay(200);

    uint16_t total = _player.getTotalAudioNumber();
    uint8_t vol = _player.getVolume();
    M5.Display.printf("Audio %d tracks vol=%d", total, vol);
    Serial.printf("Audio: %d tracks, vol=%d\n", total, vol);
    return true;
}

void AudioController::update() {
    if (!_ready) return;
    _player.update();
}

bool AudioController::play(const char *name) {
    if (!_ready) return false;
    const AudioTrack *t = find(name);
    if (!t) return false;

    size_t nameLen = strlen(t->file);
    uint8_t buf[32];
    buf[0] = 0x04;
    buf[1] = 0xFB;
    buf[2] = nameLen + 1;
    buf[3] = 0x07;
    memcpy(buf + 4, t->file, nameLen);
    uint8_t sum = 0;
    for (size_t i = 0; i < 4 + nameLen; i++) sum += buf[i];
    buf[4 + nameLen] = sum;

    while (Serial1.available()) Serial1.read();
    Serial1.write(buf, 5 + nameLen);

    uint8_t resp[16];
    uint8_t ri = 0;
    unsigned long start = millis();
    while (millis() - start < 600) {
        while (Serial1.available() && ri < sizeof(resp)) {
            resp[ri++] = Serial1.read();
            start = millis();
        }
        if (ri >= 6) {
            unsigned long gap = millis();
            while (millis() - gap < 3 && ri < sizeof(resp)) {
                if (Serial1.available()) {
                    resp[ri++] = Serial1.read();
                    gap = millis();
                }
            }
            break;
        }
        delay(1);
    }

    if (ri < 6) {
        Serial.printf("Audio: play '%s' no response (%d bytes)\n", t->file, ri);
        return false;
    }

    bool valid = false;
    uint8_t status = 0xFF;

    if (ri >= 7) {
        sum = 0;
        for (int i = 0; i < 6; i++) sum += resp[i];
        if (resp[6] == (uint8_t)sum) {
            valid = true;
            status = resp[4];
        }
    }
    if (!valid && ri >= 6) {
        sum = 0;
        for (int i = 0; i < 5; i++) sum += resp[i];
        if (resp[5] == (uint8_t)sum) {
            valid = true;
            status = resp[4];
        }
    }
    if (!valid) {
        Serial.printf("Audio: play '%s' bad frame\n", t->file);
        return false;
    }
    if (status != 0x00) {
        Serial.printf("Audio: play '%s' NAK (0x%02X)\n", t->file, status);
        return false;
    }

    delay(200);
    if (!isPlaying()) {
        Serial.printf("Audio: play '%s' accepted but not playing (bad WAV?)\n", t->file);
        return false;
    }
    Serial.printf("Audio: playing '%s'\n", t->file);
    return true;
}

bool AudioController::playByIndex(uint8_t index) {
    if (!_ready || index == 0 || index > TRACK_COUNT) return false;
    uint16_t ret = _player.playAudioByIndex(index);
    if (ret == 0xFFFF) {
        Serial.printf("Audio: playIndex %d failed\n", index);
        return false;
    }
    Serial.printf("Audio: playing idx=%d (num=%d)\n", index, ret);
    return true;
}

void AudioController::stop() {
    if (!_ready) return;
    _player.stopAudio();
}

void AudioController::pause() {
    if (!_ready) return;
    _player.pauseAudio();
}

void AudioController::resume() {
    if (!_ready) return;
    _player.playAudio();
}

void AudioController::next() {
    if (!_ready) return;
    _player.nextAudio();
}

void AudioController::prev() {
    if (!_ready) return;
    _player.previousAudio();
}

void AudioController::setVolume(uint8_t vol) {
    if (!_ready) return;
    _player.setVolume(vol);
}

uint8_t AudioController::getVolume() {
    if (!_ready) return AUDIO_DEFAULT_VOLUME;
    return _player.getVolume();
}

bool AudioController::isPlaying() {
    if (!_ready) return false;
    return _player.checkPlayStatus() == AUDIO_PLAYER_STATUS_PLAYING;
}

const AudioTrack *AudioController::find(const char *name) {
    for (uint8_t i = 0; i < TRACK_COUNT; i++) {
        if (strcmp(TRACKS[i].name, name) == 0) return &TRACKS[i];
    }
    return nullptr;
}
