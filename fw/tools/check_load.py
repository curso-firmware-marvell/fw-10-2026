#!/usr/bin/env python3
"""
check_load.py - TP4 (Bloque 4, diapo 12): "revisar la memoria y matchear con el HEX".

  python3 tools/check_load.py --hex build/fw.hex [--sym build/bl.sym --sym build/app.sym]
  python3 tools/check_load.py --hex build/fw.hex --log build/sim.log --sym build/bl.sym --sym build/app.sym

- Decodifica la vector table del BL (saltos en 0x00..0x10) y el header + vector
  table del FW (en el slot, 0x1000) y repite los chequeos que hace el BL.
- Con --log (salida de "make sim" de ../): compara palabra por palabra
  fw.hex contra lo que ../rtl/testbench.sv cargó de verdad en memory_i.
  init_memory() imprime "loading: <i> -> <dato>", donde <i> es el índice de
  PALABRA (dirección = 4*i).
- Con --sym (salida de "nm -n" de bl.elf y app.elf): traduce direcciones a
  nombres de símbolo.

Línea N de fw.hex = palabra N-1 = dirección 4*(N-1). El header del FW (0x1000)
es la línea 1025.
"""
import argparse
import re
import struct
import sys

# --- fw_image.h -------------------------------------------------------------
FW_IMAGE_MAGIC = 0x46345054          # "TP4F"
FW_HDR_SIZE = 0x18
CRC_START = 0x10
# --- project_config.def ------------------------------------------------------
RAM_START, RAM_END = 0x00020000, 0x00040000
# --- memory_map.ld (si hay --sym se toma de __fw_slot_start) ---------------
DEFAULT_SLOT = 0x00001000


def crc8(data, init=0xFF, poly=0x07):
    """El CRC-8 del TP3 (str_crc8 en lib/src/strutils.c):
    polinomio 0x07, valor inicial STR_CRC8_INIT = 0xFF. crc8({1,2,3,4}) = 0x32."""
    crc = init
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = ((crc << 1) ^ poly) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc

BL_VECTORS = [(0x00, "PROGADDR_RESET"), (0x04, ""), (0x08, ""), (0x0C, ""),
              (0x10, "PROGADDR_IRQ (IRQs deshabilitadas)")]
LOAD_RE = re.compile(r"loading:\s*([0-9a-fA-F]+)\s*->\s*(\S+)")


def read_hex(path):
    words = []
    with open(path) as f:
        for n, line in enumerate(f, 1):
            s = line.strip()
            if not s:
                continue
            try:
                words.append(int(s, 16))
            except ValueError:
                sys.exit(f"{path}:{n}: línea inválida {s!r}")
    return words


def read_log(path):
    loaded = {}
    with open(path, errors="replace") as f:
        for line in f:
            m = LOAD_RE.search(line)
            if m:
                idx, data = int(m.group(1), 16), m.group(2)
                try:
                    loaded[idx] = int(data, 16)
                except ValueError:       # x / z en la simulación
                    loaded[idx] = data
    return loaded


def read_syms(paths):
    """{addr: [(tipo, nombre, archivo), ...]} a partir de salidas de 'nm -n'"""
    syms = {}
    for p in paths or []:
        with open(p) as f:
            for line in f:
                parts = line.split()
                if len(parts) == 3:
                    try:
                        syms.setdefault(int(parts[0], 16), []).append((parts[1], parts[2], p))
                    except ValueError:
                        pass
    return syms


def find_sym(syms, name):
    for addr, lst in syms.items():
        if any(n == name for _, n, _ in lst):
            return addr
    return None


def owner(syms, name):
    """archivo .sym que define 'name' (para distinguir el del BL del del FW)"""
    addr = find_sym(syms, name)
    return None if addr is None else next(p for _, n, p in syms[addr] if n == name)


def sym(syms, addr, prefer=None):
    cands = syms.get(addr, [])
    if not cands:
        return ""
    # preferir: el binario que corresponde, código (T/t), nombres sin "__"
    best = sorted(cands, key=lambda c: (c[2] != prefer, c[0] not in "Tt",
                                        c[1].startswith("__"), c[1]))[0]
    return f" <{best[1]}>"


def decode_jal(word, pc):
    """jal rd, offset  (j offset = jal x0, offset)"""
    if word & 0x7F != 0x6F:
        return None
    rd = (word >> 7) & 0x1F
    imm = (((word >> 31) & 1) << 20 | ((word >> 12) & 0xFF) << 12 |
           ((word >> 20) & 1) << 11 | ((word >> 21) & 0x3FF) << 1)
    if imm & (1 << 20):
        imm -= 1 << 21
    return rd, (pc + imm) & 0xFFFFFFFF


def main():
    ap = argparse.ArgumentParser(description="TP4: vector tables, header del FW y fw.hex vs simulación")
    ap.add_argument("--hex", required=True, help="fw.hex de la imagen BL + FW")
    ap.add_argument("--log", help="log de make sim (líneas 'loading: ...')")
    ap.add_argument("--sym", action="append", help="salida de 'nm -n' (repetible)")
    ap.add_argument("--slot", type=lambda s: int(s, 0), help="dirección del slot del FW")
    a = ap.parse_args()

    ok = True
    words = read_hex(a.hex)
    syms = read_syms(a.sym)
    slot = a.slot or find_sym(syms, "__fw_slot_start") or DEFAULT_SLOT
    bl_syms, fw_syms = owner(syms, "_bl_vectors"), owner(syms, "__fw_header")
    print(f"[hex] {a.hex}: {len(words)} palabras (0x{0:08x}-0x{4 * len(words) - 1:08x})")

    mem = words
    if a.log:
        loaded = read_log(a.log)
        if not loaded:
            sys.exit(f"[log] {a.log}: no hay líneas 'loading:' (¿es la salida de make sim?)")
        missing = [i for i in range(len(words)) if i not in loaded]
        diff = [(i, words[i], loaded[i]) for i in range(len(words))
                if i in loaded and loaded[i] != words[i]]
        extra = sorted(i for i in loaded if i >= len(words))
        print(f"[log] {a.log}: init_memory() cargó {len(loaded)} palabras en memory_i")
        if missing or diff:
            ok = False
            for i, w, l in diff[:10]:
                got = f"{l:08x}" if isinstance(l, int) else l
                print(f"  DIFERENCIA 0x{4 * i:08x} (línea {i + 1}): fw.hex={w:08x} cargado={got}")
            if missing:
                print(f"  FALTAN {len(missing)} palabras, la primera en 0x{4 * missing[0]:08x}")
        else:
            print("[log] OK: lo que cargó el simulador coincide palabra por palabra con fw.hex")
        if extra:
            print(f"  (además cargó {len(extra)} palabra(s) después del final de fw.hex)")
        # de acá en adelante se decodifica lo que REALMENTE cargó el simulador
        mem = [loaded.get(i, w) if isinstance(loaded.get(i, w), int) else 0
               for i, w in enumerate(words)]

    def w(addr):
        i = addr // 4
        return mem[i] if 0 <= i < len(mem) else None

    print("\nVector table del BL (la ejecuta el core: son instrucciones):")
    for off, desc in BL_VECTORS:
        word = w(off)
        if word is None:
            print(f"  0x{off:08x}  (fuera de la imagen)")
            ok = False
            continue
        j = decode_jal(word, off)
        txt = (f"{'j' if j[0] == 0 else f'jal x{j[0]},'} 0x{j[1]:08x}{sym(syms, j[1], bl_syms)}"
               if j else "no es un salto (jal)")
        print(f"  0x{off:08x}  {word:08x}  {txt:<38} {desc}")
        if off == 0 and (not j or j[1] >= slot):
            ok = False
            print("    ^ ERROR: el vector de reset tiene que saltar a código del BL")

    print(f"\nHeader + vector table del FW en 0x{slot:08x} (línea {slot // 4 + 1} de fw.hex):")
    hdr = [w(slot + 4 * k) for k in range(FW_HDR_SIZE // 4)]
    if None in hdr:
        print("  la imagen termina antes del slot del FW: no hay FW cargado")
        return 1
    magic, load_addr, size, crc, sp, entry = hdr

    def check(cond, label):
        nonlocal ok
        ok &= bool(cond)
        return "OK" if cond else f"ERROR: {label}"

    data = b"".join(struct.pack("<I", x) for x in mem)
    size_ok = FW_HDR_SIZE <= size and slot + size <= len(data)
    calc = crc8(data[slot + CRC_START:slot + size]) if size_ok else None
    print(f"  +0x00 magic         0x{magic:08x} "
          f"{check(magic == FW_IMAGE_MAGIC, 'no es TP4F')}")
    print(f"  +0x04 load_addr     0x{load_addr:08x} "
          f"{check(load_addr == slot, 'linkeado para otra dirección')}")
    print(f"  +0x08 image_size    0x{size:08x} ({size} B, hasta 0x{slot + size - 1:08x}) "
          f"{check(size_ok, 'fuera de rango')}")
    print(f"  +0x0C image_crc     0x{crc:08x} (recalculado: "
          f"{'-' if calc is None else f'0x{calc:08x}'}) {check(calc == crc, 'CRC no coincide')}")
    print(f"  +0x10 initial_sp    0x{sp:08x}{sym(syms, sp, fw_syms)} "
          f"{check(RAM_START < sp <= RAM_END and sp % 16 == 0, 'fuera de RAM o no alineado a 16')}")
    print(f"  +0x14 reset_handler 0x{entry:08x}{sym(syms, entry, fw_syms)} "
          f"{check(slot + FW_HDR_SIZE <= entry < slot + size and entry % 4 == 0, 'fuera de la imagen')}")
    rh = find_sym(syms, "Reset_Handler")
    if rh is not None:
        print(f"  Reset_Handler según nm: 0x{rh:08x} {check(rh == entry, 'no coincide con el header')}")

    print("\nRESULTADO:", "OK" if ok else "HAY ERRORES")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
