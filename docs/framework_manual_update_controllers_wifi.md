# Framework Update: Controllers and Wi-Fi Scores

This update adds two optional services to PRG32:

1. `prg32_controller.c`: merges local GPIO buttons with external controller events.
2. `prg32_wifi.c`, `prg32_http_scores.c`, and `prg32_http_games.c`: expose a
   small Wi-Fi + HTTP API for game upload/runtime info and score records.

## New C functions

```c
uint32_t prg32_controller_read(void);
const char *prg32_controller_name(uint32_t bit);
void prg32_wifi_scores_init(void);
void prg32_scores_api_start(void);
int prg32_score_submit(const char *game, const char *player, uint32_t score);
```

## Assembly ABI examples

Read controller:

```asm
call prg32_input_read
andi t0, a0, PRG32_BTN_A
bnez t0, fire
```

Submit score:

```asm
la a0, game_name
la a1, player_name
li a2, 999
call prg32_score_submit
```
