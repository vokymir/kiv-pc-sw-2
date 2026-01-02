#!/usr/bin/env bash
set -euo pipefail

# config

SRC_DIR=$(realpath .)
TMP_DIR=$(mktemp -d)

echo "reconstructing in '$TMP_DIR'..."
rm -rf "$TMP_DIR"
mkdir -p "$TMP_DIR"

# where to put which file

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

# some headers belong to a directory

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

# source files

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

# copy to destination, remove prefix
# 1=where, 2=prefix, 3=files
copy_files() {
    local dest_dir="$1"
    local prefix="${2:-}"

    # forget 2 first args, so the for-loop is nicer
    shift 2
    for f in "$@"; do
        src="$SRC_DIR/$f"
        if [[ ! -f "$src" ]]; then
            echo "ER: File not found: $f" >&2
            continue
        fi
        mkdir -p "$TMP_DIR/$dest_dir"

        # remove prefix if given, handle root files
        local dest_path
        # root
        if [[ -z "$dest_dir" ]]; then
            dest_path="$TMP_DIR/${f#$prefix}"
        else
            mkdir -p "$TMP_DIR/$dest_dir"
            dest_path="$TMP_DIR/$dest_dir/${f#$prefix}"
        fi

        cp "$src" "$dest_path"
        rm "$src"
        echo "OK: $f -> $dest_path"
    done
}

# actual work

copy_files "" "" "${ROOT_FILES[@]}"
copy_files "doc" "" "${DOC_FILES[@]}"

copy_files "include" "" "${INCLUDE_FILES[@]}"

# headers in src
copy_files "src/assembler" "" "${ASSEMBLER_HEADERS[@]}"
copy_files "src/lexer" "" "${LEXER_HEADERS[@]}"
copy_files "src/parser" "" "${PARSER_HEADERS[@]}"

# src
copy_files "src/assembler" "src_assembler_" "${ASSEMBLER_SRC[@]}"
copy_files "src/core" "src_core_" "${CORE_SRC[@]}"
copy_files "src/data_structures" "src_data_structures_" "${DATA_STRUCTURES_SRC[@]}"
copy_files "src/io" "src_io_" "${IO_SRC[@]}"
copy_files "src/lexer" "src_lexer_" "${LEXER_SRC[@]}"
copy_files "src/parser" "src_parser_" "${PARSER_SRC[@]}"
copy_files "src" "src_" "${MAIN_SRC[@]}"

echo "OK: all files reconstructed in '$TMP_DIR'"

# move from temporary dir to src

rsync -a "$TMP_DIR"/ "$SRC_DIR"/

rm -rf "$TMP_DIR"

echo "OK: all files nice and back in '$SRC_DIR'"
