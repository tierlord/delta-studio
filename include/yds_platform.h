#ifndef YDS_PLATFORM_H
#define YDS_PLATFORM_H

// Standard headers that Windows.h used to pull in implicitly
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>

#ifdef _MSC_VER
#define YDS_FORCEINLINE __forceinline
#define YDS_SELECTANY __declspec(selectany)
#else
#define YDS_FORCEINLINE inline __attribute__((always_inline))
#define YDS_SELECTANY __attribute__((weak))
#endif

#ifndef _MSC_VER
// MSVC secure CRT replacements. Only the signatures used by this code base.

#include <strings.h>

inline int yds_strcat_s(char *dst, size_t size, const char *src) {
    const size_t length = strlen(dst);
    if (length >= size) return 1;

    snprintf(dst + length, size - length, "%s", src);
    return 0;
}

inline void *yds_aligned_malloc(size_t size, size_t alignment) {
    void *result = nullptr;
    if (posix_memalign(&result, alignment, size) != 0) return nullptr;
    return result;
}

#define strcpy_s(dst, size, src) snprintf((dst), (size), "%s", (src))
#define strcat_s(dst, size, src) yds_strcat_s((dst), (size), (src))
#define localtime_s(result, timer) (localtime_r((timer), (result)) == nullptr ? 1 : 0)
template <size_t N, typename... Args>
int yds_sprintf_s(char (&buffer)[N], const char *format, Args... args) {
    return snprintf(buffer, N, format, args...);
}

template <typename... Args>
int yds_sprintf_s(char *buffer, size_t size, const char *format, Args... args) {
    return snprintf(buffer, size, format, args...);
}

#define sprintf_s yds_sprintf_s
#define memcpy_s(dst, dstSize, src, count) memcpy((dst), (src), (count))
#define _aligned_malloc(size, alignment) yds_aligned_malloc((size), (alignment))
#define _aligned_free(ptr) free(ptr)
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

inline int fopen_s(FILE **file, const char *name, const char *mode) {
    *file = fopen(name, mode);
    return (*file == nullptr) ? 1 : 0;
}

#endif /* !_MSC_VER */

#endif /* YDS_PLATFORM_H */
