# PRG32 Audio Mono Sample

This example demonstrates unsigned 8-bit PCM sample playback through the mono
I2S mixer. It registers a tiny click sample from C and triggers it once per
second.

## Prepare A Real Sample

```bash
python3 tools/wav2prg32sample.py click.wav --rate 22050 --normalize --trim-silence --out build/click.raw
```

Use `tools/prg32audio_pack.py` when the sample should travel inside a `.prg32`
cartridge AUDIO block.

## Run Embedded In Firmware

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build flash monitor
```

Checkpoint: the mono speaker repeats a short click. If there is no sound, run
the checklist in `docs/audio.md`.
