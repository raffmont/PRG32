#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="build-qemu"
SDKCONFIG_DEFAULTS="sdkconfig.defaults.qemu"
FLASH_IMAGE="$BUILD_DIR/flash_image.bin"
QEMU_EFUSE="$BUILD_DIR/qemu_efuse.bin"
FLASH_SIZE=$((4 * 1024 * 1024))
GAMES_DIR="examples/games"

step() {
  echo "[STEP] $1"
}

info() {
  echo "[INFO] $1"
}

fail() {
  echo "[FAIL] $1" >&2
  exit 1
}

require_cmd() {
  local cmd="$1"
  local hint="$2"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    fail "$cmd not found. $hint"
  fi
}

ensure_idf_env() {
  step "Checking ESP-IDF environment"
  if [[ -z "${IDF_PATH:-}" ]]; then
    fail "IDF_PATH is not set. Source ESP-IDF first (for example: . \$HOME/esp-idf/export.sh)."
  fi

  require_cmd idf.py "Source ESP-IDF first (for example: . \$HOME/esp-idf/export.sh)."
  require_cmd riscv32-esp-elf-gcc "Source ESP-IDF first so the RISC-V toolchain is on PATH."
  require_cmd python3 "Install Python 3 and retry."
}

list_games() {
  step "Listing available games"
  local found=0
  local d
  for d in "$GAMES_DIR"/*; do
    if [[ -d "$d" && -f "$d/graphics/game.S" ]]; then
      basename "$d"
      found=1
    fi
  done

  if [[ "$found" -eq 0 ]]; then
    fail "No games found under $GAMES_DIR"
  fi
}

ensure_qemu_firmware() {
  step "Configuring QEMU target (esp32c3)"
  idf.py -B "$BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$SDKCONFIG_DEFAULTS" set-target esp32c3

  step "Building QEMU firmware"
  idf.py -B "$BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$SDKCONFIG_DEFAULTS" build

  [[ -f "$BUILD_DIR/PRG32.elf" ]] || fail "Missing $BUILD_DIR/PRG32.elf after build."
  info "Firmware build ready"
}

ensure_qemu_flash() {
  step "Ensuring QEMU flash image exists ($FLASH_IMAGE)"

  if [[ ! -f "$FLASH_IMAGE" ]]; then
    info "Creating 4MB QEMU flash image"
    mkdir -p "$BUILD_DIR"
    dd if=/dev/zero of="$FLASH_IMAGE" bs=1048576 count=4 >/dev/null 2>&1 || \
      fail "Failed to create $FLASH_IMAGE"
  fi

  local size
  size="$(wc -c < "$FLASH_IMAGE" | tr -d '[:space:]')"
  if [[ "$size" != "$FLASH_SIZE" ]]; then
    fail "$FLASH_IMAGE has size $size bytes, expected $FLASH_SIZE bytes (4MB)."
  fi

  info "QEMU flash image ready (4MB)"
}

ensure_qemu_efuse() {
  step "Ensuring QEMU efuse image exists ($QEMU_EFUSE)"

  if [[ ! -f "$QEMU_EFUSE" ]]; then
    python3 - "$QEMU_EFUSE" <<'PY' || fail "Failed to create $QEMU_EFUSE"
from pathlib import Path
import importlib.util
import os
import sys

out_path = Path(sys.argv[1])
qemu_ext_path = Path(os.environ["IDF_PATH"]) / "tools" / "idf_py_actions" / "qemu_ext.py"
spec = importlib.util.spec_from_file_location("qemu_ext", qemu_ext_path)
module = importlib.util.module_from_spec(spec)
assert spec and spec.loader
spec.loader.exec_module(module)
out_path.write_bytes(module.QEMU_TARGETS["esp32c3"].default_efuse)
PY
  fi

  info "QEMU efuse image ready"
}

build_cartridge() {
  local game_name="$1"
  local source="$GAMES_DIR/$game_name/graphics/game.S"
  local out="$BUILD_DIR/${game_name}.prg32"

  [[ -f "$source" ]] || fail "Game source not found: $source"

  step "Building cartridge for game '$game_name'"

  export CPATH="$BUILD_DIR/config:$BUILD_DIR${CPATH:+:$CPATH}"

  python3 tools/prg32_game.py build \
    "$source" \
    --firmware-elf "$BUILD_DIR/PRG32.elf" \
    --name "$game_name" \
    --out "$out"

  [[ -f "$out" ]] || fail "Cartridge file was not produced: $out"
  info "Cartridge built: $out"
}

stage_cartridge_to_qemu() {
  local game_name="$1"
  local cart="$BUILD_DIR/${game_name}.prg32"

  step "Staging cartridge into QEMU flash"
  python3 tools/prg32_game.py upload-qemu "$cart" --flash "$FLASH_IMAGE"

  echo
  echo "[OK] Cartridge '$game_name' loaded into $FLASH_IMAGE"
  echo "[INFO] Restart QEMU to run the newly loaded cartridge."
}

usage() {
  cat <<'USAGE'
Usage:
  ./scripts/load_cart.sh <game-name>
  ./scripts/load_cart.sh list

Examples:
  ./scripts/load_cart.sh pong
  ./scripts/load_cart.sh list
USAGE
}

main() {
  cd "$ROOT_DIR"

  if [[ "$#" -ne 1 ]]; then
    usage
    exit 1
  fi

  local arg="$1"
  if [[ "$arg" == "list" ]]; then
    list_games
    exit 0
  fi

  ensure_idf_env
  ensure_qemu_firmware
  ensure_qemu_flash
  ensure_qemu_efuse
  build_cartridge "$arg"
  stage_cartridge_to_qemu "$arg"
}

main "$@"
