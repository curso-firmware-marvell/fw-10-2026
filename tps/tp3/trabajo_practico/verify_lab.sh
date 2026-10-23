#!/usr/bin/env bash
#
# Verificación automática de la entrega del laboratorio.
# Ejecutar desde trabajo_practico/:  bash verify_lab.sh   o   make verify
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
DEMO="$ROOT/build/building_process_demo"
LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

pass=0
fail=0

ok()  { echo "  [OK] $*"; pass=$((pass + 1)); }
bad() { echo "  [FAIL] $*"; fail=$((fail + 1)); }

echo "=== Verificación del laboratorio (trabajo_practico) ==="
echo ""

echo "1. Compilación"
if make clean >/dev/null 2>&1 && make -s; then
    ok "make"
else
    bad "make (revisá SRCS, LIB_SRCS y errores de compilación)"
    echo ""
    echo "Resumen: $pass ok, $fail fallos"
    exit 1
fi

if [[ ! -x "$DEMO" ]]; then
    bad "no existe $DEMO"
    exit 1
fi
ok "ejecutable presente"

if grep -rnE '\b(printf|fprintf|puts|putchar)[[:space:]]*\(' src/app src/demo src/lib src/hal >/dev/null; then
    bad 'se usa printf/puts/putchar: imprimí solo con las funciones de print.h'
else
    ok 'salida solo mediante print.h'
fi

echo ""
echo "2. Salida del programa (especificación del TP, no la de las actividades de clase)"
"$DEMO" >"$LOG" 2>&1 || true

check() {
    if grep -qF -- "$1" "$LOG"; then ok "$2"; else bad "falta '$1'${3:+ ($3)}"; fi
}
check 'factorial(5) = 120' 'math: factorial'
check 'gcd(48, 18) = 6'    'math: gcd'
check 'value=42'           'strbuf'
check "trimmed: 'hello'"   'str_trim'
check 'crc8: 0x32'         'crc8 (init 0xFF)'      '¿copiaste las actividades de clase con init 0x00?'
check 'uart BRR=138'       'uart BRR 16 MHz'       '¿usaste los 8 MHz de las actividades de clase?'
check 'ring: ABCDEFGHIJ'   'ring buffer'
check 'timer_ms: 3'        'hal_timer'             'hitos 8 y 9'
check 'frame: ok'          'integración frame/CRC' 'hito 9'

echo ""
echo "3. Biblioteca estática (hito 11+)"
if [[ -f build/libutils.a ]]; then
    if ar t build/libutils.a | grep -q 'hal_timer.o'; then
        ok 'libutils.a contiene hal_timer.o'
    else
        bad 'libutils.a sin hal_timer.o (¿falta en LIB_SRCS?)'
    fi
else
    bad 'no hay build/libutils.a (¿USE_STATIC_LIB = 1?)'
fi

echo ""
if [[ "$fail" -eq 0 ]]; then
    echo "=== ENTREGA OK ($pass comprobaciones) ==="
    exit 0
else
    echo "=== ENTREGA INCOMPLETA: $pass ok, $fail fallos ==="
    exit 1
fi
