#include "../include/yds_windows_audio_wave_file.h"
#include <string>

#ifdef _WIN32
#include "../include/yds_windows_audio_wave_file.h"

#include <string>

ysWindowsAudioWaveFile::ysWindowsAudioWaveFile() : ysAudioFile(Platform::Windows, AudioFormat::Wave) {
    m_fileHandle = NULL;
    m_dataSegmentOffset = 0;
}

ysWindowsAudioWaveFile::~ysWindowsAudioWaveFile() {
    DestroyInternalBuffer();
    if (m_fileHandle) mmioClose(m_fileHandle, 0);
}

ysAudioFile::Error ysWindowsAudioWaveFile::OpenFile(const char *fname) {
    ysAudioFile::Error error = ysAudioFile::OpenFile(fname);
    if (error != Error::None) return error;

    char localFname[256];
    strcpy_s(localFname, 256, fname);

    // Attempt to open the file
    HMMIO file = mmioOpen(localFname, 0, MMIO_READ);

    if (file == NULL) {
        return Error::CouldNotOpenFile;
    }

    // Check that the file is a wave file
    MMCKINFO mmckinfoParent;
    MMCKINFO mmckinfoSubchunk;

    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(file, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
        // Not a waveform file
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    // Find the wave format segment
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(file, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
        // Could not find format segment
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    DWORD dwFmtSize = mmckinfoSubchunk.cksize;
    WAVEFORMATEX format;

    // Read the "FMT" chunk. 
    if (mmioRead(file, (HPSTR)(&format), dwFmtSize) != dwFmtSize) {
        // Failed to read format chunk
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    // Find the data segment
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(file, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
        // No data chunk
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    // Get size of data segment
    DWORD dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L) {
        // File contains no data
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    // Save the location within the file
    long fileOffset = mmioSeek(file, 0, SEEK_CUR);
    if (fileOffset < 0) {
        // Weird error
        mmioClose(file, 0);
        return Error::InvalidFileFormat;
    }

    // Everything worked, commit data

    m_fileHandle = file;
    m_dataSegmentOffset = fileOffset;

    m_audioParameters.m_channelCount = format.nChannels;
    m_audioParameters.m_sampleRate = format.nSamplesPerSec;
    m_audioParameters.m_bitsPerSample = (format.nBlockAlign / format.nChannels) * 8;

    m_sampleCount = dwDataSize / format.nBlockAlign;

    m_fileOpen = true;

    return Error::None;
}

ysAudioFile::Error ysWindowsAudioWaveFile::CloseFile() {
    Error error = ysAudioFile::CloseFile();
    if (error != Error::None) return error;

    mmioClose(m_fileHandle, 0);

    return Error::None;
}

ysAudioFile::Error ysWindowsAudioWaveFile::GenericRead(SampleOffset offset, SampleOffset size, void *buffer) {
    Error error = ysAudioFile::GenericRead(offset, size, buffer);
    if (error != Error::None) return error;

    long fileOffset = m_audioParameters.GetSizeFromSamples(offset) + m_dataSegmentOffset;
    long fileSize = (long)m_audioParameters.GetSizeFromSamples(size);

    mmioSeek(m_fileHandle, fileOffset, SEEK_SET);

    // Read the waveform-audio data subchunk. 
    if (mmioRead(m_fileHandle, (HPSTR)buffer, (DWORD)fileSize) != (DWORD)fileSize) {
        // Failed
        return Error::FileReadError;
    }

    return Error::None;
}

#else
ysWindowsAudioWaveFile::ysWindowsAudioWaveFile() : ysAudioFile(Platform::SDL, AudioFormat::Wave) {
    m_fileHandle = nullptr;
    m_dataSegmentOffset = 0;
}

ysWindowsAudioWaveFile::~ysWindowsAudioWaveFile() {
    DestroyInternalBuffer();
    if (m_fileHandle) fclose(m_fileHandle);
}

namespace {
    // RIFF fields are little endian regardless of the host
    uint32_t DecodeLE32(const unsigned char *bytes) {
        return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
    }

    uint16_t DecodeLE16(const unsigned char *bytes) {
        return (uint16_t)(bytes[0] | (bytes[1] << 8));
    }

    bool ReadChunkHeader(FILE *file, char *id, uint32_t *size) {
        unsigned char header[8];
        if (fread(header, 1, sizeof(header), file) != sizeof(header)) return false;

        memcpy(id, header, 4);
        *size = DecodeLE32(header + 4);

        return true;
    }
}

ysAudioFile::Error ysWindowsAudioWaveFile::OpenFile(const char *fname) {
    ysAudioFile::Error error = ysAudioFile::OpenFile(fname);
    if (error != Error::None) return error;

    FILE *file = fopen(fname, "rb");
    if (file == nullptr) return Error::CouldNotOpenFile;

    // "RIFF", size, "WAVE"
    unsigned char riff[12];
    if (fread(riff, 1, sizeof(riff), file) != sizeof(riff) ||
        memcmp(riff, "RIFF", 4) != 0 ||
        memcmp(riff + 8, "WAVE", 4) != 0)
    {
        fclose(file);
        return Error::InvalidFileFormat;
    }

    bool haveFormat = false;
    bool haveData = false;
    uint16_t channelCount = 0;
    uint16_t blockAlign = 0;
    uint32_t sampleRate = 0;
    uint32_t dataSize = 0;
    long dataOffset = 0;

    char chunkId[4];
    uint32_t chunkSize;
    while (ReadChunkHeader(file, chunkId, &chunkSize)) {
        const long chunkStart = ftell(file);

        if (memcmp(chunkId, "fmt ", 4) == 0) {
            unsigned char format[16];
            if (chunkSize < sizeof(format) || fread(format, 1, sizeof(format), file) != sizeof(format)) {
                fclose(file);
                return Error::InvalidFileFormat;
            }

            channelCount = DecodeLE16(format + 2);
            sampleRate = DecodeLE32(format + 4);
            blockAlign = DecodeLE16(format + 12);
            haveFormat = true;
        }
        else if (memcmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            dataOffset = chunkStart;
            haveData = true;
        }

        if (haveFormat && haveData) break;

        // Chunks are word aligned
        if (fseek(file, chunkStart + chunkSize + (chunkSize & 1), SEEK_SET) != 0) break;
    }

    if (!haveFormat || !haveData || dataSize == 0 || channelCount == 0 || blockAlign == 0) {
        fclose(file);
        return Error::InvalidFileFormat;
    }

    m_fileHandle = file;
    m_dataSegmentOffset = (unsigned int)dataOffset;

    m_audioParameters.m_channelCount = channelCount;
    m_audioParameters.m_sampleRate = sampleRate;
    m_audioParameters.m_bitsPerSample = (blockAlign / channelCount) * 8;

    m_sampleCount = dataSize / blockAlign;

    m_fileOpen = true;

    return Error::None;
}

ysAudioFile::Error ysWindowsAudioWaveFile::CloseFile() {
    Error error = ysAudioFile::CloseFile();
    if (error != Error::None) return error;

    if (m_fileHandle) fclose(m_fileHandle);
    m_fileHandle = nullptr;

    return Error::None;
}

ysAudioFile::Error ysWindowsAudioWaveFile::GenericRead(SampleOffset offset, SampleOffset size, void *buffer) {
    Error error = ysAudioFile::GenericRead(offset, size, buffer);
    if (error != Error::None) return error;

    const long fileOffset = (long)(m_audioParameters.GetSizeFromSamples(offset) + m_dataSegmentOffset);
    const size_t fileSize = m_audioParameters.GetSizeFromSamples(size);

    if (fseek(m_fileHandle, fileOffset, SEEK_SET) != 0) return Error::FileReadError;
    if (fread(buffer, 1, fileSize, m_fileHandle) != fileSize) return Error::FileReadError;

    return Error::None;
}
#endif
