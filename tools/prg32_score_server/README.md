# PRG32 Score Server

This optional classroom server stores scores from PRG32 boards in SQLite.

## Setup

```bash
python3 -m venv .venv
. .venv/bin/activate
pip install -r tools/prg32_score_server/requirements.txt
python tools/prg32_score_server/app.py
```

The server listens on `0.0.0.0:5000` by default.

## API

```bash
curl http://localhost:5000/api/scores
curl -X POST http://localhost:5000/api/scores \
  -H 'Content-Type: application/json' \
  -d '{"game":"pong","player":"Ada","score":42}'
```

Set `PRG32_SCORE_DB=/path/to/scores.db` to choose a database file.
