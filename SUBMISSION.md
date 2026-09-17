# GameJam 2026 Submission — copy/paste fields

Fill in team name + members yourself; everything else below is ready to paste
into the itch.io submission form.

## Game title
Degrees of Freedom: Lost on the Odyssey

## Short description (one line)
A first-person space-station escape game where you regain one movement
"Degree of Freedom" at a time by solving a different 2D puzzle in each of
5 sectors.

## Longer description (itch.io page body)
After a catastrophic failure aboard the deep-space research vessel *Odyssey*,
Rocky wakes up trapped in the lower decks with the ship's computer having
revoked every physical degree of freedom — no walking backward, no strafing,
not even the spacebar. To reach the cockpit, Rocky has to fight through 5
sectors of the ship, and in each one, solve a console's puzzle to physically
regain one more way to move.

**Theme connection:** the game *is* the theme, literally — the player's own
control scheme is the thing being unlocked, one degree of freedom per
sector, from a BRS trivia terminal, to a space-themed Flow Free puzzle, a
sliding tile puzzle, a memory-match game, and finally a Space Invaders-style
defense to retake the cockpit.

**Core loop:** walk a first-person maze corridor → find the sector's locked
console → press E to interact and solve its minigame → unlock a new
movement + a lit path through the rest of the maze → reach the exit → repeat
for the next sector, now with more ways to move.

- Randomized maze layout and puzzle solutions every run (never the same
  Flow Free board or maze twice).
- Save/continue, a leaderboard (fastest full runs), pause menu, and a full
  game-script/briefing screen.
- Everything except the two mockup UI backgrounds and the one nebula photo
  is procedurally drawn — no external sprite sheets.

## Engine, framework, and major tools used
- [Raylib](https://www.raylib.com/) 6.0 (C11), built with GCC/Clang + `make`.
- No other engine, framework, or third-party game library.

## Run instructions and controls
See [README.md](README.md) for full build/run steps per OS. Quick version:

```bash
brew install raylib   # macOS; see README for Linux/Windows-MSYS64
make run
```

Controls: Mouse to look, WASD/arrows to move (unlocked progressively), E to
interact with a console, Esc to pause, F11 for fullscreen. Full table in the
README.

## Credits
- Design, narrative, and the provided image/sound asset pack: the team.
- Code implemented with AI pair-programming assistance (Claude, Anthropic)
  from the team's design document and assets; all creative direction,
  testing, and acceptance decisions were the team's.
- Raylib 6.0 (zlib license).

## Known bugs / limitations
- Built and playtested on macOS; not build-tested on Windows or Linux.
- Maze layout and Flow Free solution are randomized per run by design —
  expect a different layout each time you play.

## GitHub repository
https://github.com/DirayatSupro/DOF-Odyssey
