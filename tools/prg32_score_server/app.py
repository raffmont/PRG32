from __future__ import annotations

import os
import sqlite3
import time
from pathlib import Path

from flask import Flask, g, jsonify, request


APP_DIR = Path(__file__).resolve().parent
DB_PATH = Path(os.environ.get("PRG32_SCORE_DB", APP_DIR / "scores.db"))

app = Flask(__name__)


def get_db() -> sqlite3.Connection:
    if "db" not in g:
        DB_PATH.parent.mkdir(parents=True, exist_ok=True)
        g.db = sqlite3.connect(DB_PATH)
        g.db.row_factory = sqlite3.Row
    return g.db


def init_db() -> None:
    db = get_db()
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS scores (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            game TEXT NOT NULL,
            player TEXT NOT NULL,
            score INTEGER NOT NULL,
            created_at INTEGER NOT NULL
        )
        """
    )
    db.execute(
        "CREATE INDEX IF NOT EXISTS scores_game_score_idx "
        "ON scores(game, score DESC, created_at ASC)"
    )
    db.commit()


@app.before_request
def before_request() -> None:
    init_db()


@app.teardown_appcontext
def close_db(error) -> None:
    db = g.pop("db", None)
    if db is not None:
        db.close()


@app.get("/api/scores")
def list_scores():
    game = request.args.get("game")
    limit = min(max(request.args.get("limit", default=20, type=int), 1), 100)
    db = get_db()
    if game:
        rows = db.execute(
            """
            SELECT game, player, score, created_at
            FROM scores
            WHERE game = ?
            ORDER BY score DESC, created_at ASC
            LIMIT ?
            """,
            (game, limit),
        ).fetchall()
    else:
        rows = db.execute(
            """
            SELECT game, player, score, created_at
            FROM scores
            ORDER BY score DESC, created_at ASC
            LIMIT ?
            """,
            (limit,),
        ).fetchall()
    return jsonify([dict(row) for row in rows])


@app.post("/api/scores")
def submit_score():
    data = request.get_json(silent=True) or {}
    game = str(data.get("game", "")).strip()[:24]
    player = str(data.get("player", "")).strip()[:24]
    try:
        score = int(data.get("score"))
    except (TypeError, ValueError):
        score = -1

    if not game or not player or score < 0:
        return jsonify({"ok": False, "error": "expected game, player, score"}), 400

    db = get_db()
    db.execute(
        """
        INSERT INTO scores(game, player, score, created_at)
        VALUES (?, ?, ?, ?)
        """,
        (game, player, score, int(time.time())),
    )
    db.commit()
    return jsonify({"ok": True})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", "5000")))
