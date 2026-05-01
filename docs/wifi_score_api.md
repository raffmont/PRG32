# URG32 Wi-Fi Score API

URG32 can expose a small REST API over Wi-Fi for game/player/score records. This is useful for classroom competitions and for teaching the boundary between low-level assembly and network services.

## Enable Wi-Fi

Edit `main/urg32_config.h`:

```c
#define URG32_WIFI_SCORES_ENABLE 1
#define URG32_WIFI_SSID "your-network"
#define URG32_WIFI_PASSWORD "your-password"
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
call urg32_score_submit
```

This is a clean ABI example: arguments in `a0`, `a1`, `a2`, return value in `a0`.

## Current implementation

The current classroom implementation stores a small in-RAM scoreboard. It is intentionally simple and easy to inspect. A production extension can persist the same records with NVS or upload them to a central course server.
