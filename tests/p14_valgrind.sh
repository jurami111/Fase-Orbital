#!/bin/sh
set -eu

cd "$(dirname "$0")/.."
if ! command -v valgrind >/dev/null 2>&1; then
    echo "P14 SKIP: Valgrind no esta instalado; ejecutar en un sistema compatible (Linux)." >&2
    exit 77
fi

make build/pruebas
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
    --errors-for-leak-kinds=all --error-exitcode=99 \
    ./build/pruebas --p14
echo "P14 PASS: cuatro bots, diez oleadas, sin fugas ni errores de Memcheck."
