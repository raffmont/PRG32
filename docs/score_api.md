# PRG32 Score API

PRG32 can expose a small REST API over Wi-Fi and can also submit records to an external classroom server. This is useful for competitions and for teaching the boundary between assembly, C, and network services.

## Enable Wi-Fi

Edit `main/prg32_config.h`:

```c
#define PRG32_WIFI_SCORES_ENABLE 1
#define PRG32_WIFI_SSID "your-network"
#define PRG32_WIFI_PASSWORD "your-password"
```

Then build and flash normally with ESP-IDF.

## API endpoints

### Get scores

```http
GET /api/scores
```

Response:

```json
[
  {"game":"pong","player":"Ada","score":42}
]
```

### Submit score

```http
POST /api/scores
Content-Type: application/json

{"game":"breakout","player":"Grace","score":1200}
```

Response:

```json
{"ok":true}
```

## Assembly usage

Assembly code should normally call a C helper at the end of a game:

```asm
la a0, game_name      # const char *game
la a1, player_name    # const char *player
li a2, 1200           # uint32_t score
call prg32_score_submit
```

This is a clean ABI example: arguments in `a0`, `a1`, `a2`, return value in `a0`.

## Remote classroom server

Run the optional Flask + SQLite server:

```bash
python3 tools/prg32_score_server/app.py
```

Then submit from a host:

```bash
curl -X POST http://localhost:5000/api/scores \
  -H 'Content-Type: application/json' \
  -d '{"game":"pong","player":"Ada","score":42}'
```

Firmware C code can submit remotely with:

```c
prg32_score_submit_remote("http://192.168.1.20:5000",
                          "pong",
                          "Ada",
                          42);
```

## Current implementation

The board-local implementation stores a small in-RAM scoreboard. The optional Python server persists records in SQLite.

The same board HTTP server also hosts the cartridge upload API when
`PRG32_GAME_UPLOAD_ENABLE` is enabled. See `docs/cartridges.md`.
