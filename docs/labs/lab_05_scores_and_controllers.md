# Lab 05 - Scores and External Controllers

## Goal

Use the score API and the UART controller bridge.

## Steps

1. Run the local server:

```bash
python3 tools/prg32_score_server/app.py
```

2. Test it from a terminal:

```bash
curl -X POST http://localhost:5000/api/scores \
  -H 'Content-Type: application/json' \
  -d '{"game":"pong","player":"Ada","score":42}'
```

3. Enable Wi-Fi scores in `main/prg32_config.h` if using board networking.
4. Submit a local score with `prg32_score_submit`.
5. Connect a bridge board or serial helper that sends:

```text
'U' 'G' <lo> <hi>
```

6. Verify that `prg32_input_read` sees both local buttons and bridge input.

## Checkpoint

Show one score in `/api/scores` and one controller movement through the bridge.

## Reflection

What part of the system translates a real USB controller into the simple PRG32 bitmask?
