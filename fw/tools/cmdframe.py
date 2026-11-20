#!/usr/bin/env python3
"""Arma tramas del protocolo del TP en el formato de los scripts de test (fw/tests/*.txt).

  cmdframe.py req  05 00 55                 ->  > A5 5A 04 00 05 00 55 <crc> C3 3C
  cmdframe.py resp 85 00                    ->  < A5 5A 03 00 85 00 <crc> C3 3C
  cmdframe.py req  05 01 --bad-crc          ->  trama con el CRC alterado
  cmdframe.py req  05 01 --bad-footer       ->  trama con el footer alterado
  cmdframe.py req  05 01 --len-delta 3      ->  LENGTH declarado +3 respecto del real
  cmdframe.py stream --len 256 --seed 1     ->  lineas del hito 1: len + datos y respuesta crc8 len

Los bytes se pasan en hexadecimal. 'req' imprime una linea '>' y 'resp' una linea '<'
(con --crc-token el CRC de la respuesta se escribe como 'crc' y lo calcula el testbench).
"""
import argparse
import random
import sys

HEADER = (0xA5, 0x5A)
FOOTER = (0xC3, 0x3C)


def crc8(data):
    """El mismo CRC-8 que str_crc8() de fw/lib: poly 0x07, valor inicial 0xFF."""
    crc = 0xFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc


def frame(opcode, payload, bad_crc=False, bad_footer=False, len_delta=0):
    """Devuelve la lista de bytes de la trama completa."""
    body = bytes([opcode]) + bytes(payload)
    crc = crc8(body)
    if bad_crc:
        crc ^= 0x5A
    length = (len(body) + 1 + len_delta) & 0xFFFF
    footer = (FOOTER[0], FOOTER[1] ^ 0xFF) if bad_footer else FOOTER
    return list(HEADER) + [length & 0xFF, length >> 8] + list(body) + [crc] + list(footer)


def hexline(prefix, data, crc_token_at=None):
    toks = ["%02X" % b for b in data]
    if crc_token_at is not None:
        toks[crc_token_at] = "crc"
    return prefix + " " + " ".join(toks)


def parse_hex(tokens):
    try:
        return [int(t, 16) & 0xFF for t in tokens]
    except ValueError:
        sys.exit("error: byte hexadecimal invalido en %s" % tokens)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("req", "resp"):
        p = sub.add_parser(name)
        p.add_argument("bytes", nargs="+", help="OPCODE y PAYLOAD en hexadecimal")
        p.add_argument("--bad-crc", action="store_true")
        p.add_argument("--bad-footer", action="store_true")
        p.add_argument("--len-delta", type=int, default=0)
        p.add_argument("--crc-token", action="store_true", help="escribir el CRC como 'crc'")
    s = sub.add_parser("stream")
    s.add_argument("--len", type=int, required=True)
    s.add_argument("--seed", type=int, default=1)
    a = ap.parse_args()

    if a.cmd in ("req", "resp"):
        raw = parse_hex(a.bytes)
        f = frame(raw[0], raw[1:], a.bad_crc, a.bad_footer, a.len_delta)
        if a.cmd == "req":
            print(hexline(">", f))
        else:
            print(hexline("<", f, len(f) - 3 if a.crc_token else None))
    else:
        rnd = random.Random(a.seed)
        data = [rnd.randrange(256) for _ in range(a.len)]
        lo, hi = a.len & 0xFF, a.len >> 8
        print("> %02X %02X" % (lo, hi))
        for i in range(0, len(data), 32):
            print(hexline(">", data[i:i + 32]))
        print("< %02X %02X %02X" % (crc8(data), lo, hi))


if __name__ == "__main__":
    main()
