#!/usr/bin/env bash
set -euo pipefail

# -----------------------------
# Configuration
# -----------------------------
FLAT_ROOT="."
DEST_ROOT="orig"

echo "📂 Reconstructing directory tree in '$DEST_ROOT'..."
rm -rf "$DEST_ROOT"
mkdir -p "$DEST_ROOT"

# -----------------------------
# Lists of files per directory
# -----------------------------

ROOT_FILES=(
    makefile
    Makefile.win
    reverse.bash
)

DOC_FILES=(
    doc.pdf
    doc.tex
)

INCLUDE_FILES=(
    args.h
    assembler.h
    codeseg.h
    common.h
    dataseg.h
    fileutil.h
    instruction.h
    lexer.h
    memory.h
    output.h
    parser.h
    symbol.h
)

ASSEMBLER_HEADERS=(
    assembler_convert.h
    assembler_pass_1.h
    assembler_pass_2.h
    assembler_passes.h
    internal.h
)

LEXER_HEADERS=(
    lexer_set_token.h
    lexer_token_array.h
)

PARSER_HEADERS=(
    parser_grammar.h
    parser_identifier.h
    parser_instruction.h
    parser_segment.h
    parser_token.h
)

ASSEMBLER_SRC=(
    src_assembler_assembler.c
    src_assembler_convert.c
    src_assembler_pass_1.c
    src_assembler_pass_2.c
    src_assembler_passes.c
)

CORE_SRC=(
    src_core_args.c
    src_core_common.c
    src_core_memory.c
)

DATA_STRUCTURES_SRC=(
    src_data_structures_codeseg.c
    src_data_structures_dataseg.c
    src_data_structures_instruction.c
    src_data_structures_symbol.c
)

IO_SRC=(
    src_io_fileutil.c
    src_io_output.c
)

LEXER_SRC=(
    src_lexer_lexer.c
    src_lexer_set_token.c
    src_lexer_token_array.c
)

PARSER_SRC=(
    src_parser_grammar.c
    src_parser_identifier.c
    src_parser_instruction.c
    src_parser_parser.c
    src_parser_segment.c
    src_parser_token.c
)

MAIN_SRC=(
    src_main.c
)

# -----------------------------
# Helper function to copy files to destination, optionally removing a prefix
# -----------------------------
copy_files() {
    local dest_dir="$1"
    local prefix="${2:-}"
    shift 2
    for f in "$@"; do
        src="$FLAT_ROOT/$f"
        if [[ ! -f "$src" ]]; then
            echo "⚠️  File not found: $f" >&2
            continue
        fi
        mkdir -p "$DEST_ROOT/$dest_dir"

        # Remove prefix if given
        local filename="${f#$prefix}"

        cp "$src" "$DEST_ROOT/$dest_dir/$filename"
        rm "$src"
        echo "✅ $f → $DEST_ROOT/$dest_dir/$filename"
    done
}

# -----------------------------
# Process all lists
# -----------------------------

# Root files
for f in "${ROOT_FILES[@]}"; do
    src="$FLAT_ROOT/$f"
    [[ -f "$src" ]] || continue
    cp "$src" "$DEST_ROOT/"
    rm "$src"
    echo "✅ $f → $DEST_ROOT/"
done

# Include headers
copy_files "include" "" "${INCLUDE_FILES[@]}"

# Module headers
copy_files "src/assembler" "" "${ASSEMBLER_HEADERS[@]}"
copy_files "src/lexer" "" "${LEXER_HEADERS[@]}"
copy_files "src/parser" "" "${PARSER_HEADERS[@]}"

# Source files with prefix removal
copy_files "src/assembler" "src_assembler_" "${ASSEMBLER_SRC[@]}"
copy_files "src/core" "src_core_" "${CORE_SRC[@]}"
copy_files "src/data_structures" "src_data_structures_" "${DATA_STRUCTURES_SRC[@]}"
copy_files "src/io" "src_io_" "${IO_SRC[@]}"
copy_files "src/lexer" "src_lexer_" "${LEXER_SRC[@]}"
copy_files "src/parser" "src_parser_" "${PARSER_SRC[@]}"
copy_files "src" "src_" "${MAIN_SRC[@]}"

# doc
copy_files "doc" "" "${DOC_FILES[@]}"

echo "🎉 All files reconstructed in '$DEST_ROOT'"
