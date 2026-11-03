#include "../include/yds_timing.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>

#include <intrin.h>

static bool qpcFlag;
static LARGE_INTEGER qpcFrequency;
#else
#include <time.h>
#endif

ysTimingSystem *ysTimingSystem::g_instance = nullptr;

uint64_t SystemTime() {
#ifdef _WIN32
    if (qpcFlag) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        currentTime.QuadPart *= 1000000;
        currentTime.QuadPart /= qpcFrequency.QuadPart;

        return (uint64_t)(currentTime.QuadPart);
    }
    else {
        // Convert output to microseconds
        return (uint64_t)(timeGetTime() * 1000);
    }
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (uint64_t)now.tv_sec * 1000000 + (uint64_t)now.tv_nsec / 1000;
#endif
}

ysTimingSystem::ysTimingSystem() {
    SetPrecisionMode(Precision::Microsecond);
    Initialize();
}

ysTimingSystem::~ysTimingSystem() {
    /* void */
}

uint64_t ysTimingSystem::GetTime() {
    return SystemTime();
}

inline uint64_t SystemClock() {
#ifdef _WIN32
    return __rdtsc();
#else
    // There is no portable cycle counter, so use a nanosecond clock instead
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
#endif
}

uint64_t ysTimingSystem::GetClock() {
    return (uint64_t)SystemClock();
}

void ysTimingSystem::SetPrecisionMode(Precision mode) {
    m_precisionMode = mode;

    if (mode == Precision::Millisecond) {
        m_div = 1000.0;
    }
    else if (mode == Precision::Microsecond) {
        m_div = 1000000.0;
    }
}

double ysTimingSystem::ConvertToSeconds(uint64_t t_u) {
    return t_u / m_div;
}

void ysTimingSystem::Update() {
    if (!m_isPaused) {
        m_frameNumber++;
    }

    uint64_t thisTime = GetTime();
    m_lastFrameDuration = (thisTime - m_lastFrameTimestamp);
    m_lastFrameTimestamp = thisTime;

    uint64_t thisClock = GetClock();
    m_lastFrameClockTicks = (thisClock - m_lastFrameClockstamp);
    m_lastFrameClockstamp = thisClock;

    if (m_frameNumber > 1) {
        if (m_averageFrameDuration <= 0) {
            m_averageFrameDuration = (double)(m_lastFrameDuration / m_div);
        }
        else {
            m_averageFrameDuration *= 0.95;
            m_averageFrameDuration += 0.05 * (double)m_lastFrameDuration / m_div;

            m_fps = (float)(1 / m_averageFrameDuration);
        }
    }
}

void ysTimingSystem::Initialize() {
#ifdef _WIN32
    qpcFlag = (QueryPerformanceFrequency(&qpcFrequency) > 0);
#endif

    m_frameNumber = 0;

    m_lastFrameTimestamp = GetTime();
    m_lastFrameDuration = 0;

    m_lastFrameClockstamp = GetClock();
    m_lastFrameClockTicks = 0;

    m_isPaused = false;

    m_averageFrameDuration = 0;
    m_fps = 0;
}

double ysTimingSystem::GetFrameDuration() {
    return m_lastFrameDuration / m_div;
}

uint64_t ysTimingSystem::GetFrameDuration_us() {
    return m_lastFrameDuration;
}
