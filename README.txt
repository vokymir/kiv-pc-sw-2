# Semester work for KIV/PC

Transcriber from custom assembly code to custom binary.
Assignment PDF in `doc/`

## How to submit

1. Run `./submit.bash`
2. Submit the new `assembler.zip`

## How it works

makefile works, just run `make`

`include/` directory has *public headers* for modules in `src/`

`src/` directory has all used modules

`tests/` have its own make, call `make` when in the subdirectory

## How to use in neovim with clangd LSP

call `make bear` to create `compile_command.json`

do that every time folder structure is altered or when LSP stops

restart LSP in nvim via `:LspRestart`

## Documentation

`doc/doc.pdf` look there, or the src is in `doc/src/doc.tex`

