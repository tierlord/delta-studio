#include "../include/os_utilities.h"

#include "../include/path.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

dbasic::Path dbasic::GetModulePath() {
    char path[256];

#ifdef _WIN32
    DWORD result = GetModuleFileName(NULL, path, 256);
#else
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    path[(length > 0) ? length : 0] = '\0';
#endif

    Path fullPath = Path(path);
    Path parentPath;

    fullPath.GetParentPath(&parentPath);

    return parentPath;
}
