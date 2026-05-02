#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QEMU_BUILD_DIR="build-qemu"
QEMU_DEFAULTS="sdkconfig.defaults.qemu"
DEMO_SOURCE="examples/games/asteroids/graphics/game.S"
DEMO_PREFIX="asteroids_graphics"
DEMO_CART="$QEMU_BUILD_DIR/asteroids.prg32"
DEMO_FLASH="$QEMU_BUILD_DIR/qemu_flash.bin"

fail() {
  echo "[FAIL] $1" >&2
  exit 1
}

info() {
  echo "[INFO] $1"
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    fail "$1 not found. $2"
  fi
}

require_cmd idf.py "Run: . \$HOME/esp-idf/export.sh"
require_cmd python3 "Install Python 3 and retry."
require_cmd riscv32-esp-elf-gcc "Run: . \$HOME/esp-idf/export.sh"

if [[ -z "${IDF_PATH:-}" ]]; then
  fail "ESP-IDF is not sourced (IDF_PATH is empty). Run: . \$HOME/esp-idf/export.sh"
fi

cd "$ROOT_DIR"

info "Configuring QEMU target (esp32c3)"
idf.py -B "$QEMU_BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$QEMU_DEFAULTS" set-target esp32c3

info "Building QEMU firmware"
idf.py -B "$QEMU_BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$QEMU_DEFAULTS" build

if [[ ! -f "$QEMU_BUILD_DIR/PRG32.elf" ]]; then
  fail "Missing $QEMU_BUILD_DIR/PRG32.elf after build"
fi

info "Building demo cartridge"
python3 tools/prg32_game.py build \
  "$DEMO_SOURCE" \
  --firmware-elf "$QEMU_BUILD_DIR/PRG32.elf" \
  --entry-prefix "$DEMO_PREFIX" \
  --name asteroids \
  --out "$DEMO_CART"

if [[ ! -f "$DEMO_FLASH" ]]; then
  fail "Missing $DEMO_FLASH. Run this once first: idf.py -B $QEMU_BUILD_DIR -D SDKCONFIG_DEFAULTS=$QEMU_DEFAULTS qemu --graphics monitor"
fi

info "Staging demo cartridge into QEMU flash"
python3 tools/prg32_game.py upload-qemu "$DEMO_CART" --flash "$DEMO_FLASH"

info "Starting QEMU screen"
exec idf.py -B "$QEMU_BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$QEMU_DEFAULTS" qemu --graphics monitor
