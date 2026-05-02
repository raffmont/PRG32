#!/usr/bin/env python3
"""Build and upload PRG32 cartridge games.

The cartridge workflow keeps the PRG32 firmware resident on the board. A game
is linked for the firmware's cartridge RAM address and PRG32 API import table,
packed as a .prg32 file, then uploaded over HTTP or staged into QEMU flash.
"""

from __future__ import annotations

import argparse
import binascii
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request


ROOT = Path(__file__).resolve().parents[1]
CART_HEADER = struct.Struct("<4sHHHHIIIIIII32s")
CART_MAGIC = b"PRG2"
CART_ABI_MAJOR = 1
CART_ABI_MINOR = 0
DEFAULT_PARTITION_TABLE = ROOT / "partitions_prg32.csv"
DEFAULT_CART_SLOT = "cart0"
FALLBACK_CART_RAM_SIZE = 32 * 1024

IMPORT_NAMES = [
    "prg32_ticks_ms",
    "prg32_input_read",
    "prg32_controller_read",
    "prg32_audio_beep",
    "prg32_console_clear",
    "prg32_console_putc",
    "prg32_console_write",
    "prg32_console_hex32",
    "prg32_gfx_clear",
    "prg32_gfx_present",
    "prg32_gfx_pixel",
    "prg32_gfx_rect",
    "prg32_gfx_text8",
    "prg32_debug_overlay_draw",
    "prg32_tile_clear",
    "prg32_tile_define",
    "prg32_tile_put",
    "prg32_tile_present",
    "prg32_sprite_hitbox",
    "prg32_sprite_draw_8x8",
    "prg32_sprite_draw_16x16",
    "prg32_score_submit",
]


def run(cmd: list[str], cwd: Path | None = None) -> str:
    try:
        result = subprocess.run(
            cmd,
            cwd=str(cwd) if cwd else None,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except FileNotFoundError as exc:
        raise SystemExit(f"missing tool: {cmd[0]}") from exc
    except subprocess.CalledProcessError as exc:
        sys.stderr.write(exc.stdout)
        sys.stderr.write(exc.stderr)
        raise SystemExit(f"command failed: {' '.join(cmd)}") from exc
    return result.stdout


def fetch_runtime(url: str) -> dict:
    endpoint = url.rstrip("/") + "/api/runtime"
    try:
        with urllib.request.urlopen(endpoint, timeout=10) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.URLError as exc:
        raise SystemExit(f"failed to read runtime from {endpoint}: {exc}") from exc


def parse_nm(text: str) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in text.splitlines():
        parts = line.split()
        if len(parts) >= 3:
            try:
                symbols[parts[2]] = int(parts[0], 16)
            except ValueError:
                continue
    return symbols


def parse_nm_sizes(text: str) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in text.splitlines():
        parts = line.split()
        if len(parts) >= 4:
            name = parts[-1]
            try:
                size = int(parts[1], 16)
            except ValueError:
                continue
            symbols[name] = size
    return symbols


def parse_partition_size(token: str) -> int:
    text = token.strip().lower()
    if not text:
        raise ValueError("empty partition size")
    mult = 1
    if text.endswith("k"):
        mult = 1024
        text = text[:-1]
    elif text.endswith("m"):
        mult = 1024 * 1024
        text = text[:-1]
    base = 16 if text.startswith("0x") else 10
    return int(text, base) * mult


def read_partition_slot(path: Path, slot: str) -> tuple[int, int]:
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise SystemExit(f"failed to read partition table {path}: {exc}") from exc

    for raw in lines:
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        cols = [c.strip() for c in line.split(",")]
        if len(cols) < 5:
            continue
        if cols[0] != slot:
            continue
        try:
            offset = parse_partition_size(cols[3])
            size = parse_partition_size(cols[4])
        except ValueError as exc:
            raise SystemExit(
                f"invalid partition values for {slot} in {path}: {exc}"
            ) from exc
        return (offset, size)

    raise SystemExit(f"partition slot '{slot}' not found in {path}")


def runtime_from_elf(path: Path, tool_prefix: str) -> dict:
    nm = parse_nm(run([tool_prefix + "nm", "-g", "--defined-only", str(path)]))
    if "prg32_cart_exec" not in nm:
        raise SystemExit("firmware ELF does not export prg32_cart_exec")
    size_nm = parse_nm_sizes(
        run([tool_prefix + "nm", "-S", "-g", "--defined-only", str(path)])
    )
    missing = [name for name in IMPORT_NAMES if name not in nm]
    if missing:
        raise SystemExit("firmware ELF is missing imports: " + ", ".join(missing))
    ram_size = size_nm.get("prg32_cart_exec")
    if not ram_size:
        ram_size = FALLBACK_CART_RAM_SIZE
        print(
            "warning: could not infer prg32_cart_exec size from ELF symbols; "
            f"using fallback cart RAM size {ram_size} bytes",
            file=sys.stderr,
        )
    return {
        "cart_load_addr": nm["prg32_cart_exec"],
        "cart_ram_size": ram_size,
        "imports": {name: nm[name] for name in IMPORT_NAMES},
    }


def write_imports(path: Path, imports: dict[str, int]) -> None:
    lines = ["/* Generated by tools/prg32_game.py. */"]
    for name in IMPORT_NAMES:
        if name not in imports:
            raise SystemExit(f"runtime import missing: {name}")
        lines.append(f"PROVIDE({name} = 0x{int(imports[name]):08x});")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_linker(path: Path, load_addr: int, init_symbol: str) -> None:
    path.write_text(
        f"""/* Generated by tools/prg32_game.py. */
OUTPUT_ARCH(riscv)
ENTRY({init_symbol})

SECTIONS
{{
    . = 0x{load_addr:08x};
    __cart_start = .;

    .text : {{
        *(.text*)
    }}

    .rodata : {{
        *(.rodata*)
    }}

    . = ALIGN(4);
    .data : {{
        *(.data*)
        *(.sdata*)
    }}

    . = ALIGN(4);
    __cart_load_end = .;

    .bss (NOLOAD) : {{
        *(.bss*)
        *(.sbss*)
        *(COMMON)
    }}

    . = ALIGN(4);
    __cart_end = .;

    /DISCARD/ : {{
        *(.comment*)
        *(.riscv.attributes*)
        *(.note*)
    }}
}}
""",
        encoding="utf-8",
    )


def detect_entries(symbols: dict[str, int], prefix: str | None) -> tuple[str, str, str]:
    if prefix:
        entries = (f"{prefix}_init", f"{prefix}_update", f"{prefix}_draw")
        missing = [name for name in entries if name not in symbols]
        if missing:
            raise SystemExit("game ELF is missing entries: " + ", ".join(missing))
        return entries

    def one_suffix(suffix: str) -> str:
        matches = sorted(
            name for name in symbols
            if name.endswith(suffix) and not name.startswith("__cart_")
        )
        if len(matches) != 1:
            raise SystemExit(
                f"could not infer a unique {suffix} entry; pass --entry-prefix"
            )
        return matches[0]

    return (one_suffix("_init"), one_suffix("_update"), one_suffix("_draw"))


def build(args: argparse.Namespace) -> None:
    if args.runtime_url:
        runtime = fetch_runtime(args.runtime_url)
    elif args.firmware_elf:
        runtime = runtime_from_elf(Path(args.firmware_elf), args.tool_prefix)
    else:
        raise SystemExit("build requires --runtime-url or --firmware-elf")

    load_addr = int(runtime["cart_load_addr"])
    if "cart_ram_size" not in runtime:
        print(
            "warning: runtime did not report cart_ram_size; "
            f"using fallback {FALLBACK_CART_RAM_SIZE} bytes",
            file=sys.stderr,
        )
    ram_size = int(runtime.get("cart_ram_size", FALLBACK_CART_RAM_SIZE))
    imports = {k: int(v) for k, v in runtime["imports"].items()}

    source = Path(args.source)
    out = Path(args.out)
    name = args.name or source.parent.parent.name or source.stem
    name_bytes = name.encode("ascii", "replace")[:31]

    with tempfile.TemporaryDirectory(prefix="prg32-cart-") as tmp_s:
        tmp = Path(tmp_s)
        obj = tmp / "game.o"
        elf = tmp / "game.elf"
        raw = tmp / "game.bin"
        imports_ld = tmp / "imports.ld"
        linker_ld = tmp / "cart.ld"

        # Compile once so nm can infer the entry prefix before final linking.
        run([
            args.tool_prefix + "gcc",
            "-x", "assembler-with-cpp",
            "-march=" + args.march,
            "-mabi=" + args.mabi,
            "-I", str(ROOT / "components/prg32/include"),
            "-I", str(ROOT / "main"),
            "-c", str(source),
            "-o", str(obj),
        ])
        obj_symbols = parse_nm(run([args.tool_prefix + "nm", "--defined-only", str(obj)]))
        init_sym, update_sym, draw_sym = detect_entries(obj_symbols, args.entry_prefix)

        write_imports(imports_ld, imports)
        write_linker(linker_ld, load_addr, init_sym)
        run([
            args.tool_prefix + "gcc",
            "-nostdlib",
            "-march=" + args.march,
            "-mabi=" + args.mabi,
            "-Wl,--no-relax",
            "-Wl,-T," + str(linker_ld),
            "-Wl,-T," + str(imports_ld),
            "-Wl,-Map," + str(tmp / "game.map"),
            str(obj),
            "-o", str(elf),
        ])
        linked_symbols = parse_nm(run([args.tool_prefix + "nm", "--defined-only", str(elf)]))
        run([args.tool_prefix + "objcopy", "-O", "binary", str(elf), str(raw)])

        code = raw.read_bytes()
        start = linked_symbols["__cart_start"]
        end = linked_symbols["__cart_end"]
        mem_size = end - start
        if mem_size <= 0 or mem_size > ram_size:
            raise SystemExit(f"cartridge needs {mem_size} bytes, runtime has {ram_size}")
        if len(code) > mem_size:
            raise SystemExit("internal error: binary is larger than cartridge memory")

        entries = {
            "init": linked_symbols[init_sym] - load_addr,
            "update": linked_symbols[update_sym] - load_addr,
            "draw": linked_symbols[draw_sym] - load_addr,
        }
        crc = binascii.crc32(code) & 0xffffffff
        header = CART_HEADER.pack(
            CART_MAGIC,
            CART_ABI_MAJOR,
            CART_ABI_MINOR,
            CART_HEADER.size,
            0,
            load_addr,
            len(code),
            mem_size,
            entries["init"],
            entries["update"],
            entries["draw"],
            crc,
            name_bytes + b"\0" * (32 - len(name_bytes)),
        )
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(header + code)

    print(
        f"built {out} name={name} load=0x{load_addr:08x} "
        f"code={len(code)} mem={mem_size}"
    )


def upload(args: argparse.Namespace) -> None:
    data = Path(args.cartridge).read_bytes()
    endpoint = args.url.rstrip("/") + "/api/games"
    request = urllib.request.Request(
        endpoint,
        data=data,
        method="POST",
        headers={"Content-Type": "application/octet-stream"},
    )
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            print(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", "replace")
        raise SystemExit(f"upload failed: HTTP {exc.code}: {body}") from exc
    except urllib.error.URLError as exc:
        raise SystemExit(f"upload failed: {exc}") from exc


def upload_qemu(args: argparse.Namespace) -> None:
    flash = Path(args.flash)
    data = Path(args.cartridge).read_bytes()
    partitions = Path(args.partitions)
    cart_offset, cart_size = read_partition_slot(partitions, args.slot)
    if len(data) > cart_size:
        raise SystemExit(
            f"cartridge is larger than {args.slot} ({cart_size} bytes from {partitions})"
        )
    if not flash.exists():
        raise SystemExit(f"QEMU flash image not found: {flash}")
    with flash.open("r+b") as f:
        f.seek(0, os.SEEK_END)
        size = f.tell()
        required = cart_offset + cart_size
        if size < required:
            raise SystemExit(
                "QEMU flash image is smaller than "
                f"{args.slot} requirements ({required} bytes needed)"
            )
        f.seek(cart_offset)
        f.write(b"\xff" * cart_size)
        f.seek(cart_offset)
        f.write(data)
    print(
        f"staged {args.cartridge} into {flash} at {args.slot} "
        f"(offset=0x{cart_offset:06x}, size={cart_size})"
    )


def runtime(args: argparse.Namespace) -> None:
    if args.url:
        print(json.dumps(fetch_runtime(args.url), indent=2, sort_keys=True))
    elif args.firmware_elf:
        print(json.dumps(runtime_from_elf(Path(args.firmware_elf), args.tool_prefix),
                         indent=2,
                         sort_keys=True))
    else:
        raise SystemExit("runtime requires --url or --firmware-elf")


def doctor(args: argparse.Namespace) -> None:
    failures = 0

    def ok(msg: str) -> None:
        print(f"[OK] {msg}")

    def fail(msg: str) -> None:
        nonlocal failures
        failures += 1
        print(f"[FAIL] {msg}")

    if shutil.which("idf.py"):
        ok("idf.py found")
    else:
        fail("idf.py missing (source ESP-IDF export.sh)")

    gcc_name = args.tool_prefix + "gcc"
    if shutil.which(gcc_name):
        ok(f"{gcc_name} found")
    else:
        fail(f"{gcc_name} missing (source ESP-IDF export.sh)")

    partitions_path = Path(args.partitions)
    if partitions_path.exists():
        ok(f"partitions file found: {partitions_path}")
        try:
            offset, size = read_partition_slot(partitions_path, args.slot)
        except SystemExit as exc:
            fail(str(exc))
        else:
            ok(
                f"partition {args.slot} parsed "
                f"(offset=0x{offset:06x}, size={size})"
            )
    else:
        fail(f"partitions file missing: {partitions_path}")

    if failures:
        raise SystemExit(1)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-prefix", default="riscv32-esp-elf-")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("runtime", help="print runtime linker information")
    p.add_argument("--url")
    p.add_argument("--firmware-elf")
    p.set_defaults(func=runtime)

    p = sub.add_parser("build", help="build a .prg32 cartridge from assembly")
    p.add_argument("source")
    p.add_argument("--out", required=True)
    p.add_argument("--name")
    p.add_argument("--entry-prefix")
    p.add_argument("--runtime-url")
    p.add_argument("--firmware-elf")
    p.add_argument("--march", default="rv32imc_zicsr_zifencei")
    p.add_argument("--mabi", default="ilp32")
    p.set_defaults(func=build)

    p = sub.add_parser("upload", help="upload a cartridge to hardware over HTTP")
    p.add_argument("cartridge")
    p.add_argument("--url", default="http://192.168.4.1")
    p.set_defaults(func=upload)

    p = sub.add_parser("upload-qemu", help="stage a cartridge into QEMU flash")
    p.add_argument("cartridge")
    p.add_argument("--flash", default="build-qemu/qemu_flash.bin")
    p.add_argument("--partitions", default=str(DEFAULT_PARTITION_TABLE))
    p.add_argument("--slot", default=DEFAULT_CART_SLOT)
    p.set_defaults(func=upload_qemu)

    p = sub.add_parser("doctor", help="check local toolchain prerequisites")
    p.add_argument("--partitions", default=str(DEFAULT_PARTITION_TABLE))
    p.add_argument("--slot", default=DEFAULT_CART_SLOT)
    p.set_defaults(func=doctor)

    args = parser.parse_args(argv)
    args.func(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
