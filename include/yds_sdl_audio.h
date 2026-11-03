#ifndef YDS_SDL_AUDIO_H
#define YDS_SDL_AUDIO_H

#include "yds_audio_system.h"
#include "yds_audio_device.h"
#include "yds_audio_source.h"
#include "yds_audio_buffer.h"

#include <SDL.h>

// SDL audio backend. It mirrors the DirectSound8 backend: every source is a
// looping ring buffer that the application fills through LockBufferSegment().
// Only 16-bit mono sources are supported, which is the format the engine uses.

class ysSDLAudioDevice;

class ysSDLAudioSystem : public ysAudioSystem {
public:
    ysSDLAudioSystem();
    virtual ~ysSDLAudioSystem();

    virtual void EnumerateDevices();
    virtual void ConnectDevice(ysAudioDevice *device, ysWindow *windowAssociation);
    virtual void DisconnectDevice(ysAudioDevice *device);
};

class ysSDLAudioDevice : public ysAudioDevice {
    friend class ysSDLAudioSystem;
    friend class ysSDLAudioSource;

public:
    ysSDLAudioDevice();
    virtual ~ysSDLAudioDevice();

    virtual ysAudioBuffer *CreateBuffer(const ysAudioParameters *parameters, SampleOffset size);

    virtual ysAudioSource *CreateSource(const ysAudioParameters *parameters, SampleOffset size);
    virtual ysAudioSource *CreateSource(ysAudioBuffer *sourceBuffer);

    virtual void UpdateAudioSources();

protected:
    ysError Open();
    void Close();

    // Keeps the audio callback away from source state while it is changed
    void LockAudio();
    void UnlockAudio();

    static void AudioCallback(void *userdata, Uint8 *stream, int length);
    void Mix(Sint16 *output, int sampleCount);

protected:
    SDL_AudioDeviceID m_sdlDevice;
};

class ysSDLAudioSource : public ysAudioSource {
    friend class ysSDLAudioDevice;

public:
    ysSDLAudioSource();
    virtual ~ysSDLAudioSource();

    virtual ysError LockEntireBuffer(void **buffer, SampleOffset *samples);
    virtual ysError UnlockBuffer(void *buffer, SampleOffset samples);
    virtual ysError LockBufferSegment(SampleOffset offset, SampleOffset samples, void **segment1, SampleOffset *size1, void **segment2, SampleOffset *size2);
    virtual ysError UnlockBufferSegments(void *segment1, SampleOffset segment1Size, void *segment2, SampleOffset segment2Size);

    virtual ysError SetMode(ysAudioSource::Mode mode = Mode::Play);

    virtual ysError SetDataBuffer(ysAudioBuffer *buffer);

    virtual SampleOffset GetCurrentPosition();
    virtual SampleOffset GetCurrentWritePosition();

    virtual void Seek(SampleOffset offset);

    virtual ysError Destroy();

protected:
    void Initialize(ysSDLAudioDevice *device, const ysAudioParameters *parameters, SampleOffset size);

    void AcquireDevice();
    void ReleaseDevice();

    // Called from the audio callback with the device lock held
    void Mix(Sint16 *output, int sampleCount);

protected:
    ysSDLAudioDevice *m_device;
    Sint16 *m_ring;
    SampleOffset m_playCursor;
    bool m_deviceLocked;
};

class ysSDLAudioBuffer : public ysAudioBuffer {
public:
    ysSDLAudioBuffer();
    virtual ~ysSDLAudioBuffer();

    virtual ysError Initialize(SampleOffset samples, const ysAudioParameters &parameters);
    virtual ysError EditBuffer(void *data);
    virtual ysError EditBufferSegment(void *data, SampleOffset samples, SampleOffset offset);
    virtual ysError Destroy();

    Sint16 *GetBuffer() const { return m_buffer; }

protected:
    Sint16 *m_buffer;
};

#endif /* YDS_SDL_AUDIO_H */
