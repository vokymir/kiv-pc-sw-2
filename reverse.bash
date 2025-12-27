#!/bin/bash
set -e

PLAIN_DIR="."
OUT_SRC="src"
OUT_INC="include"
MODULES=("assembler" "core" "data_structures" "io" "lexer" "parser")

mkdir -p "$OUT_SRC" "$OUT_INC"

# Move .c files
for f in "$PLAIN_DIR"/src_*.c; do
    [ -e "$f" ] || continue
    base=$(basename "$f" .c)
    rest="${base#src_}"
    moved=false
    for m in "${MODULES[@]}"; do
        if [[ $rest == "$m"* ]]; then
            file="${rest#${m}_}.c"
            mkdir -p "$OUT_SRC/$m"
            mv "$f" "$OUT_SRC/$m/$file" 2>/dev/null || mv "$f" "$OUT_SRC/$m/$rest.c"
            moved=true
            break
        fi
    done
    if [ "$moved" = false ]; then
        # fallback: main.c or any other non-module file
        mv "$f" "$OUT_SRC/$rest.c"
    fi
done

# Move headers
for h in "$PLAIN_DIR"/*.h; do
    [ -e "$h" ] || continue
    mv "$h" "$OUT_INC/"
done

echo "Structure restored."
