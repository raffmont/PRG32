# PRG32 Audio Stereo Pan Test

This example verifies PRG32 Audio Plus wiring with two MAX98357A boards.

Expected sequence:

1. left speaker only
2. right speaker only
3. both speakers centered

## Run

Wire both MAX98357A boards to the same BCLK, LRCLK, and DATA lines. Configure
one board for left-channel output and the other for right-channel output.

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build flash monitor
```

Checkpoint: the serial monitor labels match the speaker that plays. If left and
right are reversed, swap the board channel selection or speaker labels.
