# PRG32 Audio Mono Tracker

This example introduces tracker-style event playback. A looping waveform acts
as instrument 0, and a short event list plays a repeating three-note melody.

## Learning Goals

- register a sample as an instrument
- understand `NOTE_ON`, `NOTE_OFF`, `JUMP`, and tempo
- compare event sequencing with blocking `prg32_audio_play_notes`

## Run

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build flash monitor
```

Checkpoint: the melody loops while the main task keeps refreshing the display.
