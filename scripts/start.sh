#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="build-qemu"
GAMES_DIR="examples/games"
GAME_TOOL="tools/prg32_game.py"
QEMU_IMAGE="$BUILD_DIR/flash_image.bin"
QEMU_EFUSE="$BUILD_DIR/qemu_efuse.bin"
QEMU_ELF="$BUILD_DIR/PRG32.elf"
SDKCONFIG_DEFAULTS="sdkconfig.defaults.qemu"
FLASH_SIZE=$((4 * 1024 * 1024))

COLOR_RESET=""
COLOR_RED=""
COLOR_GREEN=""
COLOR_YELLOW=""

if [[ -t 1 ]] && command -v tput >/dev/null 2>&1; then
  if [[ "$(tput colors 2>/dev/null || echo 0)" -ge 8 ]]; then
    COLOR_RESET="$(tput sgr0)"
    COLOR_RED="$(tput setaf 1)"
    COLOR_GREEN="$(tput setaf 2)"
    COLOR_YELLOW="$(tput setaf 3)"
  fi
fi

log_info() {
  echo "${COLOR_YELLOW}[INFO]${COLOR_RESET} $1"
}

log_ok() {
  echo "${COLOR_GREEN}[OK]${COLOR_RESET} $1"
}

log_error() {
  echo "${COLOR_RED}[ERROR]${COLOR_RESET} $1" >&2
}

step() {
  echo "${COLOR_YELLOW}[INFO]${COLOR_RESET} $1"
}

die() {
  log_error "$1"
  exit 1
}

check_python() {
  if ! command -v python3 >/dev/null 2>&1; then
    die "python3 not found. Install Python 3 first (for example: brew install python)."
  fi
  log_ok "python3 detected"
}

find_idf_root() {
  local candidate

  if [[ -n "${IDF_PATH:-}" && -f "$IDF_PATH/export.sh" ]]; then
    printf '%s\n' "$IDF_PATH"
    return 0
  fi

  for candidate in "$HOME/esp-idf" "$HOME/Desktop/esp-idf"; do
    if [[ -f "$candidate/export.sh" ]]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

load_idf_env() {
  if command -v idf.py >/dev/null 2>&1 && command -v riscv32-esp-elf-gcc >/dev/null 2>&1; then
    log_ok "ESP-IDF already loaded"
    return 0
  fi

  local idf_root
  if ! idf_root="$(find_idf_root)"; then
    log_error "ESP-IDF not loaded and no local checkout was found."
    cat <<'EOF_FIX'
Run one of these commands after installing ESP-IDF:
. ~/esp-idf/export.sh
. ~/Desktop/esp-idf/export.sh
EOF_FIX
    exit 1
  fi

  log_info "Loading ESP-IDF from $idf_root"
  # shellcheck disable=SC1090
  source "$idf_root/export.sh"

  if ! command -v idf.py >/dev/null 2>&1; then
    die "ESP-IDF export loaded from $idf_root, but idf.py is still unavailable."
  fi

  if ! command -v riscv32-esp-elf-gcc >/dev/null 2>&1; then
    die "ESP-IDF export loaded from $idf_root, but riscv32-esp-elf-gcc is still unavailable."
  fi

  log_ok "ESP-IDF environment loaded"
}

check_idf_tools() {
  if ! command -v qemu-system-riscv32 >/dev/null 2>&1; then
    log_error "qemu-system-riscv32 not found."
    cat <<'EOF_FIX'
Run:
python "$IDF_PATH/tools/idf_tools.py" install qemu-riscv32
. ~/esp-idf/export.sh
EOF_FIX
    exit 1
  fi

  log_ok "ESP-IDF tools detected"
}

check_qemu_runtime_deps() {
  local qemu_output
  if ! qemu_output="$(qemu-system-riscv32 -machine help 2>&1)"; then
    if [[ "$qemu_output" == *"libslirp"* ]]; then
      log_error "qemu-system-riscv32 failed due to missing libslirp."
      cat <<'EOF_FIX'
Run:
brew install libslirp
EOF_FIX
      exit 1
    fi
    die "qemu-system-riscv32 failed to start: $qemu_output"
  fi
  log_ok "qemu-system-riscv32 runtime check passed"
}

validate_project_layout() {
  [[ -f "$GAME_TOOL" ]] || die "Missing $GAME_TOOL. Run this script from the PRG32 repository root."
  [[ -d "$GAMES_DIR" ]] || die "Missing $GAMES_DIR. Run this script from the PRG32 repository root."

  if [[ ! -d "$BUILD_DIR" ]]; then
    log_info "$BUILD_DIR is missing; creating it"
    mkdir -p "$BUILD_DIR"
  fi

  log_ok "Project layout looks valid"
}

ensure_qemu_firmware() {
  if [[ -f "$QEMU_ELF" ]]; then
    log_ok "QEMU firmware already built ($QEMU_ELF)"
    return
  fi

  log_info "QEMU firmware missing; running initial build"
  idf.py -B "$BUILD_DIR" -D "SDKCONFIG_DEFAULTS=$SDKCONFIG_DEFAULTS" set-target esp32c3
  idf.py -B "$BUILD_DIR" build

  [[ -f "$QEMU_ELF" ]] || die "Build finished but $QEMU_ELF is still missing."
  log_ok "QEMU firmware build complete"
}

generate_qemu_flash_image() {
  step "Generating QEMU flash image"
  (
    cd "$BUILD_DIR"
    python3 -m esptool --chip=esp32c3 merge_bin --output=flash_image.bin --fill-flash-size=4MB @flash_args
  ) || die "Failed to generate $QEMU_IMAGE"

  local size
  size="$(wc -c < "$QEMU_IMAGE" | tr -d '[:space:]')"
  if [[ "$size" != "$FLASH_SIZE" ]]; then
    die "$QEMU_IMAGE has invalid size ($size bytes). Expected $FLASH_SIZE bytes (4MB)."
  fi

  log_ok "QEMU flash image ready"
}

ensure_qemu_efuse_image() {
  if [[ -f "$QEMU_EFUSE" ]]; then
    log_ok "QEMU efuse image already present ($QEMU_EFUSE)"
    return
  fi

  step "Generating QEMU efuse image"
  python3 - "$QEMU_EFUSE" <<'PY' || die "Failed to generate $QEMU_EFUSE"
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

  log_ok "QEMU efuse image ready"
}

discover_games() {
  GAMES=()

  local d
  for d in "$GAMES_DIR"/*; do
    if [[ -d "$d" ]]; then
      GAMES+=("$(basename "$d")")
    fi
  done

  if [[ "${#GAMES[@]}" -eq 0 ]]; then
    die "No games found in $GAMES_DIR"
  fi
}

print_game_menu() {
  local i

  echo
  echo "Available games:"
  for i in "${!GAMES[@]}"; do
    printf "[%d] %s\n" "$((i + 1))" "${GAMES[$i]}"
  done
  echo
  echo "Type a number to run a game"
  echo "Type r to rerun last game"
  echo "Type q to quit"
}

build_cartridge() {
  local game_name="$1"
  local source="$GAMES_DIR/$game_name/graphics/game.S"
  local out="$BUILD_DIR/${game_name}.prg32"

  [[ -f "$source" ]] || die "Missing game source: $source"

  log_info "Building cartridge: $game_name"
  CPATH="$BUILD_DIR/config:$BUILD_DIR${CPATH:+:$CPATH}" \
    python3 "$GAME_TOOL" build \
      "$source" \
      --firmware-elf "$QEMU_ELF" \
      --name "$game_name" \
      --out "$out"

  [[ -s "$out" ]] || die "Cartridge build failed: $out was not created"
  log_ok "Cartridge created: $out"
}

inject_cartridge() {
  local game_name="$1"
  local cart="$BUILD_DIR/${game_name}.prg32"

  [[ -f "$cart" ]] || die "Missing cartridge file: $cart"

  log_info "Injecting cartridge into QEMU flash"
  python3 "$GAME_TOOL" upload-qemu "$cart" --flash "$QEMU_IMAGE"
  log_ok "Cartridge staged in QEMU flash"
}

launch_qemu() {
  log_info "Launching QEMU"
  log_info "Press Ctrl + ] to exit"
  exec qemu-system-riscv32 \
    -M esp32c3 \
    -m 4M \
    -drive "file=$QEMU_IMAGE,if=mtd,format=raw" \
    -drive "file=$QEMU_EFUSE,if=none,format=raw,id=efuse" \
    -global "driver=nvram.esp32c3.efuse,property=drive,value=efuse" \
    -global "driver=timer.esp32c3.timg,property=wdt_disable,value=true" \
    -nic user,model=open_eth \
    -display sdl \
    -serial mon:stdio
}

run_game_flow() {
  local game_name="$1"

  build_cartridge "$game_name"
  generate_qemu_flash_image
  ensure_qemu_efuse_image
  inject_cartridge "$game_name"
  launch_qemu
}

main_loop() {
  local last_game=""
  local input=""
  local idx=""

  while true; do
    discover_games
    print_game_menu

    if [[ -n "$last_game" ]]; then
      printf "Selection (last: %s): " "$last_game"
    else
      printf "Selection: "
    fi

    IFS= read -r input

    case "$input" in
      q|Q)
        log_info "Goodbye"
        exit 0
        ;;
      r|R)
        if [[ -z "$last_game" ]]; then
          log_error "No previously run game. Select a game number first."
          continue
        fi
        run_game_flow "$last_game"
        ;;
      ''|*[!0-9]*)
        log_error "Invalid input: $input"
        ;;
      *)
        idx="$((input - 1))"
        if (( idx < 0 || idx >= ${#GAMES[@]} )); then
          log_error "Invalid selection number: $input"
          continue
        fi
        last_game="${GAMES[$idx]}"
        run_game_flow "$last_game"
        ;;
    esac
  done
}

main() {
  cd "$ROOT_DIR"

  log_info "PRG32 QEMU launcher starting"
  check_python
  load_idf_env
  check_idf_tools
  check_qemu_runtime_deps
  validate_project_layout
  ensure_qemu_firmware
  main_loop
}

main "$@"
