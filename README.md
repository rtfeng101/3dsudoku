# sudoku

A sudoku game for the 3DS/2DS. Builds as both a `.3dsx` (Homebrew Launcher)
and a `.cia` (HOME menu). Set up from the `hello-cia` template. See
`../hello-cia/README.md` for how the build and CIA packaging work.

## Build commands

| Command | What you get |
|---|---|
| `make` | `sudoku.3dsx` |
| `make cia` | `sudoku.3dsx` **and** `sudoku.cia` |
| `make clean` | deletes all build output |

Quick test over Wi-Fi: open the Homebrew Launcher, press **Y**, then run
`make && 3dslink sudoku.3dsx`.

## Still to do

- Replace `meta/icon.png` (48x48), `meta/banner.png` (256x128) and
  `meta/audio.wav`. These are still the hello-cia artwork.
- Bump `CIA_VERSION_MINOR` in the `Makefile` each time you install an update
  over an older version.
