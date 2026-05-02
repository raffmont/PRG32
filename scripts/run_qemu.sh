#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="build-qemu"
SDKCONFIG_DEFAULTS="sdkconfig.defaults.qemu"
FLASH_IMAGE="$BUILD_DIR/flash_image.bin"
QEMU_EFUSE="$BUILD_DIR/qemu_efuse.bin"
FLASH_SIZE=$((4 * 1024 * 1024))

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

build_qemu_firmware() {
  step "Configuring QEMU target (esp32c3)"
  idf.py -B "$BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$SDKCONFIG_DEFAULTS" set-target esp32c3

  step "Building QEMU firmware"
  idf.py -B "$BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$SDKCONFIG_DEFAULTS" build

  [[ -f "$BUILD_DIR/PRG32.elf" ]] || fail "Missing $BUILD_DIR/PRG32.elf after build."
  info "Firmware build ready"
}

generate_flash_image() {
  step "Generating QEMU flash image"
  (
    cd "$BUILD_DIR"
    python3 -m esptool --chip=esp32c3 merge_bin --output=flash_image.bin --fill-flash-size=4MB @flash_args
  ) || fail "Failed to generate $FLASH_IMAGE"
}

run_qemu() {
  step "Starting QEMU"
  echo
  echo "Press Ctrl + ] to exit QEMU monitor."
  echo
  exec qemu-system-riscv32 \
    -M esp32c3 \
    -m 4M \
    -drive "file=$FLASH_IMAGE,if=mtd,format=raw" \
    -drive "file=$QEMU_EFUSE,if=none,format=raw,id=efuse" \
    -global "driver=nvram.esp32c3.efuse,property=drive,value=efuse" \
    -global "driver=timer.esp32c3.timg,property=wdt_disable,value=true" \
    -nic user,model=open_eth \
    -display sdl \
    -serial mon:stdio
}

main() {
  cd "$ROOT_DIR"
  ensure_idf_env
  build_qemu_firmware
  ensure_qemu_flash
  ensure_qemu_efuse
  generate_flash_image
  run_qemu
}

main "$@"
