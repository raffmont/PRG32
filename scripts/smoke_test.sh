#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

ok() {
  echo "[OK] $1"
}

fail() {
  echo "[FAIL] $1"
  exit 1
}

warn() {
  echo "[WARN] $1"
}

if ! command -v python3 >/dev/null 2>&1; then
  fail "python3 not found (install Python 3 and retry)"
fi

if ! command -v idf.py >/dev/null 2>&1; then
  fail "idf.py not found (run: source ESP-IDF export.sh)"
fi

if ! command -v riscv32-esp-elf-gcc >/dev/null 2>&1; then
  warn "riscv32-esp-elf-gcc not found (run: source ESP-IDF export.sh)"
fi

run_step() {
  local name="$1"
  shift
  if "$@"; then
    ok "$name"
  else
    fail "$name"
  fi
}

run_step "doctor" python3 tools/prg32_game.py doctor

run_step "set-target-esp32c3" \
  idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3

run_step "build-firmware" \
  idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build

if [[ ! -f build-qemu/PRG32.elf ]]; then
  fail "build-firmware (missing build-qemu/PRG32.elf)"
fi
ok "firmware-artifact"

run_step "build-demo-cartridge" \
  python3 tools/prg32_game.py build \
    examples/games/pong/graphics/game.S \
    --firmware-elf build-qemu/PRG32.elf \
    --entry-prefix pong_graphics \
    --name pong \
    --out build-qemu/pong.prg32

if [[ ! -f build-qemu/pong.prg32 ]]; then
  fail "build-demo-cartridge (missing build-qemu/pong.prg32)"
fi
ok "cartridge-artifact"

if python3 - <<'PY'
import subprocess
import time
import sys

cmd = [
    "idf.py",
    "-B", "build-qemu",
    "-D", "SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu",
    "qemu",
    "--graphics",
    "monitor",
]

p = subprocess.Popen(cmd)
try:
    time.sleep(8)
    if p.poll() is not None:
        raise SystemExit(f"qemu exited early with code {p.returncode}")
finally:
    if p.poll() is None:
        p.terminate()
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            p.kill()
            p.wait(timeout=5)
PY
then
  ok "qemu-launch-check"
else
  warn "qemu-launch-check failed (non-blocking)"
fi

if [[ ! -f build-qemu/qemu_flash.bin ]]; then
  fail "qemu-flash-image missing (run: idf.py -B build-qemu qemu --graphics monitor once)"
fi
ok "qemu-flash-image"

run_step "stage-cartridge-qemu" \
  python3 tools/prg32_game.py upload-qemu \
    build-qemu/pong.prg32 \
    --flash build-qemu/qemu_flash.bin

ok "smoke-test-complete"
echo "=== SMOKE TEST PASSED ==="
echo "Firmware build, cartridge build, and QEMU staging are functional."
