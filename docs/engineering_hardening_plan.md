# PRG32 Engineering Hardening Plan

This plan turns PRG32 from a promising teaching prototype into a maintainable engineering project.

## Goals

- Make builds reproducible
- Make failures easy to diagnose
- Keep the educational runtime simple and readable
- Protect the cartridge ABI from accidental breakage
- Enable safe contributions from students

## Quality gates

Every pull request should pass:

1. Repository structure check
2. Python syntax check
3. Shell script syntax check
4. C/C++ formatting check
5. ESP-IDF build check when ESP-IDF is available
6. Cartridge build smoke test
7. QEMU staging smoke test when QEMU artifacts are available

## Suggested CI stages

### Stage 1 — Always available

- Verify required files and directories
- Run Python compile check on `tools/*.py`
- Run shell syntax check on `scripts/*.sh`
- Check Markdown links where possible

### Stage 2 — Toolchain available

- Build QEMU target
- Build at least one demo cartridge
- Run `tools/prg32_game.py doctor`

### Stage 3 — Nightly or release only

- Full ESP-IDF build matrix
- Cartridge compatibility checks
- Example build sweep
- Release artifact generation

## Formatting policy

- C/C++: `clang-format`
- Python: `black` or `ruff format`
- Shell: `shfmt`
- Markdown: consistent headings, no local absolute paths

Formatting should be automatic and deterministic.

## ABI stability

`components/prg32/include/prg32.h` is the public contract. Changes to this file should be reviewed carefully.

Recommended policy:

- Additive API changes are allowed within the same minor version
- Breaking API changes require ABI version bump
- Cartridge header fields must remain packed and documented
- Example cartridges should continue to build after API changes

## Test strategy

### Unit-level tests

- Cartridge header parsing
- CRC validation
- Input mask helpers
- Sprite hitbox logic
- Score serialization

### Integration tests

- Build firmware
- Build cartridge
- Stage cartridge into QEMU flash image
- Verify runtime metadata endpoint when enabled

### Regression tests

- Known good cartridges
- Known invalid cartridges
- Oversized cartridges
- ABI mismatch cartridges

## Release checklist

Before tagging a release:

- CI green
- Smoke test passes
- README updated
- CHANGELOG updated
- Examples build
- Teaching docs versioned
- Citation metadata checked
- Release notes include known limitations

## Near-term engineering backlog

1. Replace current README with `docs/README_REPLACEMENT.md`
2. Promote `docs/gold_standard_main.c` into `main/main.c` after local build validation
3. Add formatting configuration files
4. Add pre-commit hooks
5. Refactor `tools/prg32_game.py` into testable modules
6. Add unit tests for cartridge utilities
7. Add CHANGELOG.md
8. Add issue and PR templates
9. Add release workflow
10. Add example compatibility matrix
