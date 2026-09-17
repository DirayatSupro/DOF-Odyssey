# Degrees of Freedom: Lost on the Odyssey

A 5-sector first-person space-station escape game built with [Raylib](https://www.raylib.com/) 6.0 in C11.
Rocky wakes up trapped aboard the *Odyssey* with all movement locked down. Solve each
sector's console minigame to regain one Degree of Freedom, navigate the short maze it
reveals, and reach the cockpit.

## Requirements

- A C compiler (gcc/clang) and `make`.
- Raylib **5.0 or 6.0** installed and discoverable by the compiler/linker.
  - macOS: `brew install raylib`
  - Linux: install `raylib` from your package manager, or build it from source and `sudo make install`.
  - Windows (MSYS64/MinGW): `pacman -S mingw-w64-x86_64-raylib` inside the MSYS2 MinGW64 shell.

## Build & run

```bash
make        # builds build/odyssey (or build/odyssey.exe on Windows)
make run    # builds, then launches it
make clean  # removes the build/ directory
```

The `Makefile` auto-detects macOS, Linux, and Windows (MSYS64/MinGW) and links the
right system libraries for each. On macOS it looks up Raylib via `brew --prefix raylib`
automatically.

### VS Code

Just open the `odyssey/` folder and run `make run` in the integrated terminal (or wire
up a simple task that runs `make run`). No project-specific VS Code configuration is
required.

### Running the executable directly

The game resolves `assets/` and `saves/` relative to whichever of these exists first:
the current working directory, the executable's own directory, or one/two levels
above it. That means it works whether you launch it with `make run`, by double clicking
`build/odyssey`, or by running it from an IDE — as long as the `assets/` folder from
this repo is still somewhere near the binary.

## Controls

| Action | Key |
| --- | --- |
| Look around | Mouse |
| Move forward / backward | W/S or Up/Down (unlocked progressively) |
| Strafe left / right | A/D or Left/Right (unlocked progressively) |
| Interact with a console | E |
| Pause / resume | Esc |
| Toggle fullscreen | F11 |
| Minigame-specific actions | shown on screen in each minigame |

The window is freely resizable and F11 toggles borderless fullscreen at your
monitor's native resolution. Everything is drawn to a fixed 1280x720 virtual
canvas that then gets scaled and letterboxed to fit whatever size the real
window ends up at, so every screen keeps its exact layout (and the 3D view
keeps a correct, undistorted aspect ratio) no matter what size or shape
monitor you're on.

Each sector's console is locked behind a short "no degrees of freedom yet" foyer.
Walk up to it, press E, and solve that sector's minigame to unlock the next movement
capability and light up the path through the rest of that sector's maze.

## Project layout

```
src/            game source (see the top of each file for what it owns)
  minigames/    the five 2D challenges (trivia, flow free, sliding tile, card match, space invaders)
assets/         images and sounds used at runtime (copied from the provided asset pack)
saves/          created at runtime: save.dat, leaderboard.dat, settings.cfg (not committed)
```

## Known limitations / what still needs a human pass

- I could compile and smoke-test this on macOS only. The Makefile has the standard
  conditional flags for Linux and Windows-MSYS64, but I have not built or run it on
  either platform myself.
- The maze levels are deliberately simple hand-authored corridors (per the design
  brief's "no need to make complex mazes"), not open, branching mazes.
- This has been checked for a clean, warning-free build and reviewed carefully for
  logic bugs, but a full five-sector human playtest (difficulty, pacing, feel) is
  still worth doing before you call it final.

## Publishing to itch.io

I can't log into or publish to your itch.io account for you (that needs your own
credentials), but here's how to ship the build once you're happy with it:

1. `make clean && make` for a fresh release build.
2. Zip up the `build/odyssey` (or `.exe`) binary together with the `assets/` folder,
   keeping the same relative layout (binary next to `assets/`).
3. On itch.io: create a new project → upload the zip → check "This file will be played
   in the browser" **off** (it's a native build) → tag the appropriate platform
   (Windows/macOS/Linux) for the zip you made on that platform.
4. If you want command-line uploads later, itch.io's `butler` tool
   (`butler push <zip> <user>/<game>:<channel>`) can automate step 3 once you're
   logged in with `butler login`.
