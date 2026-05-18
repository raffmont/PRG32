# PRG32 Audio Mono Beep

This example verifies the mandatory mono PRG32 audio path with one generated
square-wave sample. No cartridge audio assets are required.

## Run Embedded In Firmware

1. Copy `examples/audio_mono_beep/main.c` into `main/main.c`, or temporarily add
   it as the only app source in `main/CMakeLists.txt`.
2. Wire one MAX98357A breakout as described in `docs/audio.md`.
3. Build and flash:

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build flash monitor
```

Checkpoint: one speaker plays a short beep, then the display keeps refreshing.

## Cartridge Use

This is an app-level hardware smoke test rather than a cartridge. For
cartridge audio assets, use `tools/prg32audio_pack.py` and the game examples.
