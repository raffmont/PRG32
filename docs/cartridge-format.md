# PRG32 Cartridge Format

PRG32 cartridges are native RV32 code packages loaded by the resident firmware.
The base package remains compatible with earlier PRG32 cartridges, and audio
assets are stored in an optional trailing AUDIO block.

## Base Header

The `.prg32` header starts with magic `PRG2` and stores:

- ABI major and minor version
- header size
- flags
- cartridge load address
- code size
- memory size
- init/update/draw offsets
- code payload CRC32
- cartridge name

`PRG32_CART_FLAG_AUDIO_BLOCK` marks a cartridge that has a trailing AUDIO block.

## Payload

The code payload is linked for `prg32_cart_exec`, copied into executable RAM,
and called by the resident firmware.

## Optional AUDIO Block

When present, the AUDIO block follows immediately after the code payload:

```text
PRG2 header
code/data payload
AUD0 audio block
```

The AUDIO block header stores offsets to:

- sample descriptors
- instrument descriptors
- track descriptors
- tracker events
- raw sample bytes

## Sample Descriptor

```c
typedef struct {
    uint32_t offset;
    uint32_t length;
    uint32_t loop_start;
    uint32_t loop_end;
    uint16_t base_note;
    uint8_t flags;
    uint8_t reserved;
} prg32_sample_desc_t;
```

Flag bit 0 enables looping. Source sample bytes are unsigned 8-bit PCM mono.

## Instrument Descriptor

```c
typedef struct {
    uint16_t sample_id;
    uint8_t default_volume;
    int8_t default_pan;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
} prg32_instrument_desc_t;
```

The ADSR fields are reserved for future envelope lessons.

## Track Events

```c
typedef struct {
    uint8_t delta_ticks;
    uint8_t command;
    uint8_t arg0;
    uint8_t arg1;
} prg32_audio_event_t;
```

Commands include `NOTE_ON`, `NOTE_OFF`, `SET_VOLUME`, `SET_PAN`, `SET_TEMPO`,
`PLAY_SAMPLE`, `JUMP`, and `END`.

## Asset Packing Pipeline

```bash
python3 tools/wav2prg32sample.py input.wav --rate 22050 --out build/input.raw
python3 tools/prg32audio_pack.py audio.json --out build/audio.block
python3 tools/prg32_game.py build game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix mygame \
  --audio-block build/audio.block \
  --out build-esp32c6/mygame.prg32
```

The firmware loads AUDIO assets before calling the cartridge init function, so
student code can trigger sample id `0` immediately when the block defines it.
