#!/usr/bin/env python3
"""
mkimage.py - TP4 (Bloque 4): arma la imagen combinada BL + FW para el simulador.

    python3 tools/mkimage.py --bl build/bl.bin --app build/app.bin -o build/fw.bin [--corrupt]

1. Lee el header del FW (primeros 0x18 bytes de app.bin, formato en fw_image.h).
2. Verifica magic, load_addr y que image_size coincida con el tamaño de app.bin.
3. Calcula el CRC-8 del TP3 (str_crc8: polinomio 0x07, inicial 0xFF) de
   [0x10, image_size) y lo escribe en el header (0x0C).
4. Arma fw.bin = BL | relleno con 0x00 hasta load_addr | FW firmado.
   El relleno en cero no es casual: 0x00000000 es una instrucción ilegal en
   RISC-V, así que si el PC cae ahí PicoRV32 hace trap y la simulación corta.

Después el Makefile convierte fw.bin -> fw.hex con el mismo hexdump que usa
../fw/Makefile (una palabra de 32 bits por línea): eso es lo que carga
init_memory() de ../rtl/testbench.sv desde ../fw/fw.hex.
"""
import argparse
import struct
import sys

# --- Tiene que coincidir con fw_image.h ------------------------------------
FW_IMAGE_MAGIC = 0x46345054          # "TP4F"
HDR_FMT = "<6I"                      # magic, load_addr, image_size, image_crc, initial_sp, reset_handler
HDR_SIZE = struct.calcsize(HDR_FMT)  # 0x18 = FW_HDR_SIZE
OFF_CRC = 0x0C                       # FW_HDR_IMAGE_CRC
CRC_START = 0x10                     # FW_IMAGE_CRC_START (desde la vector table)
# --- Tiene que coincidir con project_config.def -----------------------------
INST_MEM_SIZE = 0x20000              # ADDR_SIZE_INST (memory_i en rtl/mem_mux.v)


def crc8(data, init=0xFF, poly=0x07):
    """El CRC-8 del TP3 (str_crc8 en lib/src/strutils.c):
    polinomio 0x07, valor inicial STR_CRC8_INIT = 0xFF. crc8({1,2,3,4}) = 0x32."""
    crc = init
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = ((crc << 1) ^ poly) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc


def die(msg):
    sys.exit(f"mkimage: ERROR: {msg}")


def main():
    ap = argparse.ArgumentParser(description="Arma fw.bin = bootloader + FW firmado (TP4)")
    ap.add_argument("--bl", required=True, help="binario del bootloader (bl.bin)")
    ap.add_argument("--app", required=True, help="binario del FW (app.bin)")
    ap.add_argument("-o", "--out", required=True, help="imagen combinada (fw.bin)")
    ap.add_argument("--corrupt", action="store_true",
                    help="prueba negativa: invierte un bit del FW DESPUÉS de calcular el CRC")
    a = ap.parse_args()

    with open(a.bl, "rb") as f:
        bl = f.read()
    with open(a.app, "rb") as f:
        app = bytearray(f.read())

    if len(app) < HDR_SIZE:
        die(f"{a.app} es más chico que el header ({len(app)} < {HDR_SIZE} bytes): "
            "¿app.ld ubica .fw_header primero y con KEEP?")
    magic, load_addr, size, _, sp, entry = struct.unpack_from(HDR_FMT, app, 0)
    if magic != FW_IMAGE_MAGIC:
        die(f"magic 0x{magic:08x} != 0x{FW_IMAGE_MAGIC:08x}: "
            "¿el header de app_startup.S está completo y app.ld lo pone primero (KEEP)?")
    if size != len(app):
        die(f"image_size del header ({size}) != tamaño de {a.app} ({len(app)}); "
            "¿hay secciones cargables fuera de .text/.data? Mirá app.map")
    if load_addr % 4:
        die(f"load_addr 0x{load_addr:08x} no está alineada a 4")
    if len(bl) > load_addr:
        die(f"el BL ({len(bl)} bytes) pisa el slot del FW (0x{load_addr:08x})")
    if load_addr + size > INST_MEM_SIZE:
        die(f"el FW (0x{load_addr:08x} + {size} bytes) no entra en la memoria de instrucciones")

    crc = crc8(app[CRC_START:size])
    struct.pack_into("<I", app, OFF_CRC, crc)

    if a.corrupt:
        i = CRC_START + (size - CRC_START) // 2
        app[i] ^= 0x01
        print(f"mkimage: --corrupt: bit 0 invertido en 0x{load_addr + i:08x} "
              "(después de calcular el CRC: el BL lo tiene que rechazar)")

    img = bytearray(bl) + bytes(load_addr - len(bl)) + app
    img += bytes(-len(img) % 4)          # múltiplo de 4: hexdump saca palabras de 32 bits
    with open(a.out, "wb") as f:
        f.write(img)

    def line(addr):                      # línea (1-based) de fw.hex que contiene 'addr'
        return addr // 4 + 1

    print(f"mkimage: {a.out}: {len(img)} bytes = {len(img) // 4} palabras (= líneas de fw.hex)")
    print(f"  BL     0x{0:08x}-0x{len(bl) - 1:08x} {len(bl):7d} B   fw.hex líneas "
          f"{line(0)}-{line(len(bl) - 1)}")
    if load_addr > len(bl):
        print(f"  relleno 0x{len(bl):08x}-0x{load_addr - 1:08x} {load_addr - len(bl):6d} B   "
              "(0x00000000 = instrucción ilegal)")
    print(f"  FW     0x{load_addr:08x}-0x{load_addr + size - 1:08x} {size:7d} B   fw.hex líneas "
          f"{line(load_addr)}-{line(load_addr + size - 1)}")
    print(f"  header FW: crc8=0x{crc:02x}  initial_sp=0x{sp:08x}  reset_handler=0x{entry:08x}")


if __name__ == "__main__":
    main()
