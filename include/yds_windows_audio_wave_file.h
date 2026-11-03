#ifndef YDS_WINDOWS_AUDIO_WAVE_FILE_H
#define YDS_WINDOWS_AUDIO_WAVE_FILE_H

#include "yds_audio_file.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <stdio.h>
#endif

class ysWindowsAudioWaveFile : public ysAudioFile {
public:
    ysWindowsAudioWaveFile();
    ~ysWindowsAudioWaveFile();

    virtual Error OpenFile(const char *fname);
    virtual Error CloseFile();

protected:
    virtual Error GenericRead(SampleOffset offset, SampleOffset size, void *buffer);

#ifdef _WIN32
    HMMIO m_fileHandle;
#else
    FILE *m_fileHandle;
#endif
    unsigned int m_dataSegmentOffset;
};

#endif /* YDS_WINDOWS_AUDIO_WAVE_FILE_H */
