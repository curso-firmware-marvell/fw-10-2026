#!/usr/bin/env bash
#
# test_binutils.sh — Exercises the main GNU binutils against
# artifacts built from this project.
#
# Usage:  ./test_binutils.sh            (run all tests)
#         ./test_binutils.sh <tool>      (run only one section, e.g. "nm", "objdump")
#         ./test_binutils.sh --help      (show available tools)
#
set -uo pipefail

BUILD=build
DEMO="$BUILD/binutils_demo"
DEMO_DBG="$BUILD/binutils_demo_debug"
DEMO_OPT="$BUILD/binutils_demo_opt"
DEMO_STRIP="$BUILD/binutils_demo_stripped"
STATIC_LIB="$BUILD/libutils.a"
MAIN_OBJ="$BUILD/main.o"
MATH_OBJ="$BUILD/mathlib.o"
STR_OBJ="$BUILD/strutils.o"
CPP_OBJ="$BUILD/cpp_wrapper.o"
ASM_OBJ="$BUILD/asm_example.o"

COLOR_TITLE="\033[1;36m"
COLOR_CMD="\033[1;33m"
COLOR_RESET="\033[0m"

section() {
    echo ""
    echo -e "${COLOR_TITLE}================================================================${COLOR_RESET}"
    echo -e "${COLOR_TITLE}  $1${COLOR_RESET}"
    echo -e "${COLOR_TITLE}================================================================${COLOR_RESET}"
    echo ""
}

run_cmd() {
    echo -e "${COLOR_CMD}\$ $*${COLOR_RESET}"
    eval "$@" 2>&1 || true
    echo ""
}

show_help() {
    echo ""
    echo "Usage: bash test_binutils.sh [TOOL|--help]"
    echo ""
    echo "Available GNU binutils to test:"
    echo ""
    echo "  1.  as        - Assembler"
    echo "  2.  ld        - Linker"
    echo "  3.  nm        - Symbol lister"
    echo "  4.  objdump   - Object file inspector"
    echo "  5.  readelf   - ELF analyzer"
    echo "  6.  ar        - Archive manager"
    echo "  7.  ranlib    - Archive indexer"
    echo "  8.  strip     - Symbol stripper"
    echo "  9.  objcopy   - Object copier/translator"
    echo "  10. size      - Section size reporter"
    echo "  11. strings   - String extractor"
    echo "  12. addr2line - Address-to-source mapper"
    echo "  13. c++filt   - C++ name demangler"
    echo ""
    echo "Examples:"
    echo "  bash test_binutils.sh          # run all tests"
    echo "  bash test_binutils.sh nm       # run only the nm section"
    echo "  bash test_binutils.sh objdump  # run only the objdump section"
    echo ""
}

should_run() {
    [[ -z "${FILTER:-}" ]] || [[ "${FILTER}" == "$1" ]]
}

FILTER="${1:-}"

if [[ "$FILTER" == "--help" || "$FILTER" == "-h" ]]; then
    show_help
    exit 0
fi

# ── Build first ──────────────────────────────────────────────────
section "BUILDING THE PROJECT"
run_cmd make clean
run_cmd make all

# ══════════════════════════════════════════════════════════════════
# 1. as — The GNU Assembler
# ══════════════════════════════════════════════════════════════════
if should_run "as"; then
section "1. as — GNU Assembler"
cat <<'INFO'
'as' translates assembly language source files into object files.
We assembled asm_example.s during the build.  Let's verify the result.
INFO
run_cmd file "$ASM_OBJ"
run_cmd "as --version 2>/dev/null | head -1"
fi

# ══════════════════════════════════════════════════════════════════
# 2. ld — The GNU Linker
# ══════════════════════════════════════════════════════════════════
if should_run "ld"; then
section "2. ld — GNU Linker"
cat <<'INFO'
'ld' combines object files and libraries into an executable.
Normally invoked via the compiler driver; here we show its version
and the linker map for our demo executable.
INFO
run_cmd "ld --version 2>/dev/null | head -1"
run_cmd "g++ -Wl,--print-map -Wl,--cref $MAIN_OBJ $ASM_OBJ $CPP_OBJ -L$BUILD -lutils -o /dev/null 2>&1 | head -80"
fi

# ══════════════════════════════════════════════════════════════════
# 3. nm — List Symbols
# ══════════════════════════════════════════════════════════════════
if should_run "nm"; then
section "3. nm — List Symbols from Object Files"
cat <<'INFO'
'nm' lists the symbols (functions, variables) defined in object files
and executables.  Symbol types: T=text(code), D=data, B=bss, U=undefined...
INFO

echo "--- Symbols in main.o (plain C — no mangling) ---"
run_cmd nm "$MAIN_OBJ"

echo "--- Symbols in mathlib.o (plain C) ---"
run_cmd nm "$MATH_OBJ"

echo "--- Symbols in cpp_wrapper.o (C++ — mangled names) ---"
run_cmd nm "$CPP_OBJ" | head -40

echo "--- Same C++ symbols, demangled ---"
run_cmd nm --demangle "$CPP_OBJ" | head -40

echo "--- Defined symbols in the static library ---"
run_cmd nm --defined-only "$STATIC_LIB"

echo "--- Undefined (external) symbols in main.o ---"
run_cmd nm --undefined-only "$MAIN_OBJ"

echo "--- Symbols sorted by size ---"
run_cmd nm --size-sort --demangle "$DEMO"
fi

# ══════════════════════════════════════════════════════════════════
# 4. objdump — Display Object File Information
# ══════════════════════════════════════════════════════════════════
if should_run "objdump"; then
section "4. objdump — Object File Inspector"
cat <<'INFO'
'objdump' displays detailed information about object files:
headers, disassembly, relocations, symbol tables, sections, etc.
INFO

echo "--- File headers ---"
run_cmd objdump -f "$DEMO"

echo "--- Section headers ---"
run_cmd objdump -h "$DEMO"

echo "--- Disassemble the math_factorial function ---"
run_cmd objdump -d "$MATH_OBJ" | sed -n '/<math_factorial/,/^$/p'

echo "--- Disassemble asm_example.o ---"
run_cmd objdump -d "$ASM_OBJ"

echo "--- Disassemble C++ wrapper (mangled vs demangled) ---"
echo "Mangled:"
run_cmd "objdump -d $CPP_OBJ | head -30"
echo "Demangled:"
run_cmd "objdump -d --demangle $CPP_OBJ | head -30"

echo "--- Relocations in main.o ---"
run_cmd objdump -r "$MAIN_OBJ" | head -40

echo "--- Dynamic relocations in the executable ---"
run_cmd objdump -R "$DEMO" | head -30

echo "--- Display source interleaved with disassembly (debug build) ---"
run_cmd objdump -S "$DEMO_DBG" | head -80
fi

# ══════════════════════════════════════════════════════════════════
# 5. readelf — Display ELF Information
# ══════════════════════════════════════════════════════════════════
if should_run "readelf"; then
section "5. readelf — ELF File Analyzer"
cat <<'INFO'
'readelf' displays information about ELF (Executable and Linkable Format)
files.  Unlike objdump, it doesn't depend on the BFD library.
INFO

echo "--- ELF header ---"
run_cmd readelf -h "$DEMO"

echo "--- Section headers ---"
run_cmd readelf -S "$DEMO"

echo "--- Program headers (segments) ---"
run_cmd readelf -l "$DEMO"

echo "--- Symbol table ---"
run_cmd readelf -s "$DEMO" | head -40

echo "--- Dynamic section ---"
run_cmd readelf -d "$DEMO"

echo "--- Notes ---"
run_cmd readelf -n "$DEMO"

echo "--- Relocation entries ---"
run_cmd readelf -r "$DEMO" | head -30
fi

# ══════════════════════════════════════════════════════════════════
# 6. ar — Archive Manager
# ══════════════════════════════════════════════════════════════════
if should_run "ar"; then
section "6. ar — Static Library (Archive) Manager"
cat <<'INFO'
'ar' creates, modifies, and extracts from archive files (.a).
Static libraries are ar archives of .o files.
INFO

echo "--- List contents of libutils.a ---"
run_cmd ar t "$STATIC_LIB"

echo "--- Verbose table of contents ---"
run_cmd ar tv "$STATIC_LIB"

echo "--- Create a new archive with just mathlib.o ---"
run_cmd ar rcs "$BUILD/libmath_only.a" "$MATH_OBJ"
run_cmd ar t "$BUILD/libmath_only.a"

echo "--- Extract members to temp dir ---"
TMPDIR=$(mktemp -d)
run_cmd "cd $TMPDIR && ar x $(pwd)/$STATIC_LIB && ls -la && cd -"
rm -rf "$TMPDIR"
fi

# ══════════════════════════════════════════════════════════════════
# 7. ranlib — Generate Archive Index
# ══════════════════════════════════════════════════════════════════
if should_run "ranlib"; then
section "7. ranlib — Archive Index Generator"
cat <<'INFO'
'ranlib' generates an index to the contents of an archive, speeding
up linking.  Equivalent to 'ar s'.
INFO

run_cmd cp "$STATIC_LIB" "$BUILD/libutils_noindex.a"
echo "--- Before ranlib (strip index with 'ar d') ---"
run_cmd nm -s "$BUILD/libutils_noindex.a" | head -20

echo "--- Run ranlib ---"
run_cmd ranlib "$BUILD/libutils_noindex.a"

echo "--- After ranlib ---"
run_cmd nm -s "$BUILD/libutils_noindex.a" | head -20
fi

# ══════════════════════════════════════════════════════════════════
# 8. strip — Discard Symbols
# ══════════════════════════════════════════════════════════════════
if should_run "strip"; then
section "8. strip — Remove Symbols from Binaries"
cat <<'INFO'
'strip' removes symbol table and debugging information from object
files and executables, reducing file size.
INFO

echo "--- Size comparison ---"
run_cmd ls -la "$DEMO_OPT" "$DEMO_STRIP"

echo "--- Symbols in optimized (not stripped) ---"
run_cmd nm "$DEMO_OPT" | wc -l
echo "(total symbol count above)"

echo "--- Symbols in stripped version ---"
run_cmd nm "$DEMO_STRIP"
echo "(should show 'no symbols' error)"

echo "--- Selective strip: remove debug info only ---"
run_cmd cp "$DEMO_DBG" "$BUILD/demo_strip_debug_only"
run_cmd strip --strip-debug "$BUILD/demo_strip_debug_only"
run_cmd ls -la "$DEMO_DBG" "$BUILD/demo_strip_debug_only"
fi

# ══════════════════════════════════════════════════════════════════
# 9. objcopy — Copy and Translate Object Files
# ══════════════════════════════════════════════════════════════════
if should_run "objcopy"; then
section "9. objcopy — Object File Copier/Translator"
cat <<'INFO'
'objcopy' copies object files, optionally transforming them
(changing format, keeping only some sections).
INFO

echo "--- Extract .text section to raw binary ---"
run_cmd objcopy -O binary -j .text "$DEMO" "$BUILD/text_section.bin"
run_cmd ls -la "$BUILD/text_section.bin"
run_cmd xxd "$BUILD/text_section.bin" | head -20
fi

# ══════════════════════════════════════════════════════════════════
# 10. size — Section Sizes
# ══════════════════════════════════════════════════════════════════
if should_run "size"; then
section "10. size — Display Section Sizes"
cat <<'INFO'
'size' lists the size of each section (text, data, bss) and the total
size of object files or executables.
INFO

echo "--- Section sizes of all object files ---"
run_cmd size "$MAIN_OBJ" "$MATH_OBJ" "$STR_OBJ" "$CPP_OBJ" "$ASM_OBJ"

echo "--- Section sizes of all executables ---"
run_cmd size "$DEMO" "$DEMO_DBG" "$DEMO_OPT" "$DEMO_STRIP"

echo "--- Berkeley format (default) vs SysV format ---"
run_cmd size --format=sysv "$DEMO"
fi

# ══════════════════════════════════════════════════════════════════
# 11. strings — Print Readable Strings
# ══════════════════════════════════════════════════════════════════
if should_run "strings"; then
section "11. strings — Extract Readable Strings"
cat <<'INFO'
'strings' prints sequences of printable characters found in binary
files.  Useful for finding embedded messages, paths, version info, etc.
INFO

echo "--- Strings in the demo executable (min length 10) ---"
run_cmd strings -n 10 "$DEMO" | head -40

echo "--- Strings in stripped binary (still readable!) ---"
run_cmd strings -n 10 "$DEMO_STRIP" | head -40

echo "--- Strings with file offsets ---"
run_cmd strings -t x -n 15 "$DEMO" | head -20

echo "--- Compare string counts: debug vs stripped ---"
echo "Debug build:    $(strings "$DEMO_DBG" | wc -l) strings"
echo "Optimized:      $(strings "$DEMO_OPT" | wc -l) strings"
echo "Stripped:        $(strings "$DEMO_STRIP" | wc -l) strings"
fi

# ══════════════════════════════════════════════════════════════════
# 12. addr2line — Address to Source Mapping
# ══════════════════════════════════════════════════════════════════
if should_run "addr2line"; then
section "12. addr2line — Map Addresses to Source Lines"
cat <<'INFO'
'addr2line' translates program addresses into file names and line
numbers.  Requires debug info (-g).  Invaluable for crash analysis.
INFO

echo "--- Find address of 'main' symbol ---"
MAIN_ADDR=$(nm "$DEMO_DBG" | grep ' T main$' | awk '{print $1}')
if [[ -n "$MAIN_ADDR" ]]; then
    echo "Address of main: 0x$MAIN_ADDR"
    run_cmd addr2line -e "$DEMO_DBG" -f -C "$MAIN_ADDR"
fi

echo "--- Map multiple addresses ---"
ADDRS=$(nm "$DEMO_DBG" | grep ' T ' | head -5 | awk '{print $1}')
for addr in $ADDRS; do
    run_cmd addr2line -e "$DEMO_DBG" -f -C "$addr"
done

echo "--- addr2line on stripped binary (no debug info) ---"
if [[ -n "$MAIN_ADDR" ]]; then
    run_cmd addr2line -e "$DEMO_STRIP" -f "$MAIN_ADDR"
fi
fi

# ══════════════════════════════════════════════════════════════════
# 13. c++filt — Demangle C++ Symbols
# ══════════════════════════════════════════════════════════════════
if should_run "c++filt"; then
section "13. c++filt — C++ Symbol Demangler"
cat <<'INFO'
'c++filt' converts mangled C++ symbol names back to their original
human-readable form.  Our C++ wrapper (cpp_wrapper.o) produces
mangled symbols, while the plain C modules do not.  This section
contrasts both and shows how c++filt works.
INFO

echo "--- C symbols pass through unchanged ---"
run_cmd "nm $MATH_OBJ | grep ' T ' | awk '{print \$3}' | c++filt"

echo "--- C++ mangled symbols from cpp_wrapper.o ---"
MANGLED=$(nm "$CPP_OBJ" | grep ' T ' | awk '{print $3}')
echo "$MANGLED"
echo ""

echo "--- Same symbols demangled via c++filt ---"
echo "$MANGLED" | c++filt
echo ""

echo "--- Pipe full nm output through c++filt ---"
run_cmd "nm $CPP_OBJ | c++filt | head -30"

echo "--- Side-by-side: mangled vs demangled (defined T symbols) ---"
nm "$CPP_OBJ" | grep ' T ' | while read -r addr type sym; do
    demangled=$(echo "$sym" | c++filt)
    printf "  %-50s -> %s\n" "$sym" "$demangled"
done
echo ""

echo "--- Mixed C and C++ symbols in the final executable ---"
run_cmd "nm $DEMO | grep -E ' T (math_|bit_|run_cpp|_ZN3hal)' | c++filt | head -20"
fi

echo ""
echo "Done! Build artifacts are in the '$BUILD/' directory."
echo "Run with --help to see available tools."
