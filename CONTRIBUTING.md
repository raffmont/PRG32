# Contributing to PRG32

## Fork And Branch

1. Fork this repository on GitHub.
2. Clone your fork locally.
3. Create a topic branch from `main`:

```bash
git checkout -b feat/my-change
```

## Local Validation

Run the relevant subset before opening a PR:

```bash
git diff --check
PYTHONPYCACHEPREFIX=/tmp/pycache python3 -m py_compile tools/prg32_score_server/app.py
PYTHONPYCACHEPREFIX=/tmp/pycache python3 -m py_compile tools/prg32_game.py
```

If ESP-IDF is installed:

```bash
idf.py build
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build
```

## Pull Request Process

1. Keep PRs focused and small.
2. Describe:
   - problem
   - solution
   - validation steps and results
3. Link related issues/lab tasks if present.
4. Include screenshots for UI/graphics-visible changes.
5. Request review only after local checks pass.

## Coding Guidelines

- Keep the educational style: explicit code over clever abstractions.
- Preserve naming:
  - public functions: `prg32_*`
  - public constants/macros: `PRG32_*`
- Keep assembly examples classroom-friendly:
  - `.option norelax`
  - save/restore `ra` around C calls
  - keep 16-byte stack alignment
  - use game-specific symbol names (`pong_graphics_*`, `asteroids_ascii_*`, etc.)
- Do not reintroduce legacy names (`urg32` paths/symbols).
- Keep docs in sync when APIs, paths, or workflows change.

## Scope Expectations

- Default firmware app should stay minimal unless a PR explicitly changes product behavior.
- Example games in `examples/games` should remain external to default app builds.
- Cartridge/QEMU workflows must remain compatible with docs and tools.
