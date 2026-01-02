#!/usr/bin/env bash
set -euo pipefail

# -----------------------------
# Configuration
# -----------------------------
ZIPNAME="assembler.zip"
PDF="doc/doc.pdf"
DOC_TEX="doc/src/doc.tex"
DOC_PARTS="doc/src/partials/"
SRC_DIR="src"
INC_DIR="include"
REVERSE_SRC="reconstruct.bash"
MAKEFILE_SRC="makefile_for_kekstein"
TMPDIR="tmp_submit"

# -----------------------------
# Clean previous files
# -----------------------------
rm -rf "$TMPDIR" "$ZIPNAME"

echo "Creating submission archive..."

# -----------------------------
# Create temp directory
# -----------------------------
mkdir -p "$TMPDIR"

# -----------------------------
# Copy basic files
# -----------------------------
cp "$REVERSE_SRC" "$TMPDIR/reverse.bash"
cp "$PDF" "$TMPDIR/doc.pdf"
cp "$DOC_TEX" "$TMPDIR/doc.tex"

# -----------------------------
# Copy and flatten .c files
# -----------------------------
find "$SRC_DIR" -maxdepth 5 -type f -name '*.c' | while read -r f; do
    g=$(echo "$f" | tr '/' '_')
    cp "$f" "$TMPDIR/$g"
done

# -----------------------------
# Copy header files
# -----------------------------
find "$SRC_DIR" -maxdepth 5 -type f -name '*.h' -exec cp {} "$TMPDIR/" \;

# -----------------------------
# Copy include folder
# -----------------------------
cp -r "$INC_DIR"/* "$TMPDIR/"

# -----------------------------
# Copy Makefile
# -----------------------------
cp "$MAKEFILE_SRC" "$TMPDIR/makefile"

# -----------------------------
# Create Windows variant symlink
# -----------------------------
cd "$TMPDIR"
ln -s makefile Makefile.win
cd ..

# -----------------------------
# Create zip archive
# -----------------------------
cd "$TMPDIR"
zip -r "../$ZIPNAME" *
cd ..

# -----------------------------
# Cleanup temp directory
# -----------------------------
rm -rf "$TMPDIR"

echo "Created $ZIPNAME"
