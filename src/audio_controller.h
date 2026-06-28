#pragma once

#include <unit_audioplayer.hpp>

struct AudioTrack {
    const char *name;
    const char *file;
};

class AudioController {
public:
    AudioController();

    bool begin();
    void update();
    void setReady(bool r) { _ready = r; }
    bool isReady() const { return _ready; }

    bool play(const char *name);
    bool playByIndex(uint8_t index);
    void stop();
    void pause();
    void resume();
    void next();
    void prev();
    void setVolume(uint8_t vol);
    uint8_t getVolume();
    bool isPlaying();

    static const AudioTrack *find(const char *name);
    static constexpr uint8_t TRACK_COUNT = 8;
    static const AudioTrack TRACKS[];

private:
    AudioPlayerUnit _player;
    bool _ready;
};
