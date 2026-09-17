#include "respath.h"
#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

static char buffer[1024];

static bool TryJoin(const char *base, const char *rel, char *out, size_t outSize) {
    if (base == NULL) return false;
    snprintf(out, outSize, "%s%s%s", base, (strlen(base) > 0 && base[strlen(base) - 1] != '/') ? "/" : "", rel);
    return FileExists(out);
}

const char *ResolvePath(const char *relativePath) {
    // 1) as-is / relative to current working directory
    if (FileExists(relativePath)) {
        snprintf(buffer, sizeof(buffer), "%s", relativePath);
        return buffer;
    }

    const char *exeDir = GetApplicationDirectory();

    // 2) next to the executable
    if (TryJoin(exeDir, relativePath, buffer, sizeof(buffer))) return buffer;

    // 3) one directory up from the executable (build/odyssey -> project root)
    char parent[900];
    snprintf(parent, sizeof(parent), "%s../", exeDir);
    if (TryJoin(parent, relativePath, buffer, sizeof(buffer))) return buffer;

    // 4) two directories up, just in case of a deeper build layout
    char grandparent[900];
    snprintf(grandparent, sizeof(grandparent), "%s../../", exeDir);
    if (TryJoin(grandparent, relativePath, buffer, sizeof(buffer))) return buffer;

    // Fall back to the plain relative path so the caller gets a sane error message.
    snprintf(buffer, sizeof(buffer), "%s", relativePath);
    return buffer;
}

static void EnsureDirExists(const char *dirPath) {
#if defined(_WIN32)
    mkdir(dirPath);
#else
    mkdir(dirPath, 0755);
#endif
}

const char *ResolveWritablePath(const char *relativePath) {
    const char *exeDir = GetApplicationDirectory();
    char base[900];
    snprintf(base, sizeof(base), "%s../", exeDir);

    char savesDir[950];
    snprintf(savesDir, sizeof(savesDir), "%ssaves", base);
    EnsureDirExists(savesDir);

    snprintf(buffer, sizeof(buffer), "%s%s%s", base, (strlen(base) > 0 && base[strlen(base) - 1] != '/') ? "/" : "", relativePath);
    return buffer;
}
