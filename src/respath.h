#ifndef RESPATH_H
#define RESPATH_H

// Resolves a path like "assets/images/start.png" or "saves/save.dat" to an
// existing file on disk, trying (in order) the current working directory,
// the executable's directory, and the executable's parent directory. This
// keeps the game runnable whether launched via `make run`, VS Code, or by
// double-clicking the binary inside build/. Returns a pointer to an internal
// static buffer (not thread-safe, not safe to hold across two calls at once).
const char *ResolvePath(const char *relativePath);

// Same resolution rules, but for a path that may not exist yet (used before
// writing a save file). Ensures the parent "saves" directory exists at the
// chosen base and returns the path to write to.
const char *ResolveWritablePath(const char *relativePath);

#endif
