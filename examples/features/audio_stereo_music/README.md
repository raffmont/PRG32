# PRG32 Audio Stereo Music

This PRG32 Audio Plus example keeps a simple melody centered while sound
effects alternate between left and right.

## Run

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build flash monitor
```

Checkpoint: the melody appears in the center image and short effects jump
between left and right speakers.
