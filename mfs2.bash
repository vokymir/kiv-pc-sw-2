#!/bin/bash
set -ex

ARCHIVE="$PWD/javok.zip"

TMPDIR=$(mktemp -d)

echo "Copying files to temporary directory..."

cp mfk2 "$TMPDIR/makefile"

cp -r include "$TMPDIR/"
cp -r src "$TMPDIR/"

mkdir -p "$TMPDIR/doc"
cd doc
git ls-files | rsync -R --files-from=- ./ "$TMPDIR/doc/"
cd ..
ln -s "$TMPDIR/doc/doc.pdf" "$TMPDIR/doc.pdf"

# Create the zip archive
echo "Creating archive $ARCHIVE..."
(cd "$TMPDIR" && zip -r "$ARCHIVE" .)

# Clean up
rm -rf "$TMPDIR"

echo "Archive created: $ARCHIVE"
