# Contributing to PRG32

## Development principles

- Keep code readable (no minified one-line files)
- Prefer clarity over micro-optimizations (educational project)
- Document all public APIs

## Workflow

1. Fork repository
2. Create feature branch
3. Run smoke test: `./scripts/smoke_test.sh`
4. Submit PR

## Code style

- C: follow ESP-IDF style
- Python: PEP8
- Shell: POSIX compatible

## Testing

- Always run smoke test
- Prefer adding small reproducible examples
