# Framework Update: Controllers and Wi-Fi Scores

This update adds two optional services to URG32:

1. `urg32_controller.c`: merges local GPIO buttons with external controller events.
2. `urg32_wifi_scores.c`: exposes a small HTTP API for game/player/score records.

## New C functions

```c
uint32_t urg32_controller_read(void);
const char *urg32_controller_name(uint32_t bit);
void urg32_wifi_scores_init(void);
void urg32_scores_api_start(void);
int urg32_score_submit(const char *game, const char *player, uint32_t score);
```

## Assembly ABI examples

Read controller:

```asm
call urg32_input_read
andi t0, a0, URG32_BTN_A
bnez t0, fire
```

Submit score:

```asm
la a0, game_name
la a1, player_name
li a2, 999
call urg32_score_submit
```
