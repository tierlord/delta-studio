#include "../include/yds_sdl_audio.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace {
    // Samples written this far ahead of the play cursor are always safe to
    // overwrite. This is the write lead reported by GetCurrentWritePosition().
    const SampleOffset WriteLeadSamples = 2048;

    const int OutputSampleRate = 44100;
    const int OutputSamplesPerCallback = 1024;

    Sint16 ClampSample(int sample) {
        if (sample > INT16_MAX) return INT16_MAX;
        if (sample < INT16_MIN) return INT16_MIN;
        return (Sint16)sample;
    }
}

ysSDLAudioSystem::ysSDLAudioSystem() : ysAudioSystem(API::SDL) {
    /* void */
}

ysSDLAudioSystem::~ysSDLAudioSystem() {
    /* void */
}

void ysSDLAudioSystem::EnumerateDevices() {
    ysAudioSystem::EnumerateDevices();

    // SDL exposes a default output device, which is what the engine uses
    m_devices.NewGeneric<ysSDLAudioDevice>();
}

void ysSDLAudioSystem::ConnectDevice(ysAudioDevice *device, ysWindow *windowAssociation) {
    ysAudioSystem::ConnectDevice(device, windowAssociation);

    ysSDLAudioDevice *sdlDevice = static_cast<ysSDLAudioDevice *>(device);
    if (sdlDevice->Open() != ysError::None) {
        fprintf(stderr, "Could not open the SDL audio device: %s\n", SDL_GetError());
    }
}

void ysSDLAudioSystem::DisconnectDevice(ysAudioDevice *device) {
    ysSDLAudioDevice *sdlDevice = static_cast<ysSDLAudioDevice *>(device);
    sdlDevice->Close();

    ysAudioSystem::DisconnectDevice(device);
}

ysSDLAudioDevice::ysSDLAudioDevice() : ysAudioDevice(API::SDL) {
    m_sdlDevice = 0;
}

ysSDLAudioDevice::~ysSDLAudioDevice() {
    Close();
}

ysError ysSDLAudioDevice::Open() {
    YDS_ERROR_DECLARE("Open");

    if (m_sdlDevice != 0) return YDS_ERROR_RETURN(ysError::None);

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return YDS_ERROR_RETURN(ysError::CouldNotOpenAudioDevice);

    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;
    SDL_zero(desired);

    desired.freq = OutputSampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = OutputSamplesPerCallback;
    desired.callback = ysSDLAudioDevice::AudioCallback;
    desired.userdata = this;

    // Without the allowed-changes flags SDL converts to the hardware format itself
    m_sdlDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_sdlDevice == 0) return YDS_ERROR_RETURN(ysError::CouldNotOpenAudioDevice);

    SDL_PauseAudioDevice(m_sdlDevice, 0);

    return YDS_ERROR_RETURN(ysError::None);
}

void ysSDLAudioDevice::Close() {
    if (m_sdlDevice == 0) return;

    SDL_CloseAudioDevice(m_sdlDevice);
    m_sdlDevice = 0;
}

void ysSDLAudioDevice::LockAudio() {
    if (m_sdlDevice != 0) SDL_LockAudioDevice(m_sdlDevice);
}

void ysSDLAudioDevice::UnlockAudio() {
    if (m_sdlDevice != 0) SDL_UnlockAudioDevice(m_sdlDevice);
}

void ysSDLAudioDevice::AudioCallback(void *userdata, Uint8 *stream, int length) {
    ysSDLAudioDevice *device = static_cast<ysSDLAudioDevice *>(userdata);
    device->Mix(reinterpret_cast<Sint16 *>(stream), length / (int)sizeof(Sint16));
}

void ysSDLAudioDevice::Mix(Sint16 *output, int sampleCount) {
    memset(output, 0, sizeof(Sint16) * sampleCount);

    const int sourceCount = m_audioSources.GetNumObjects();
    for (int i = 0; i < sourceCount; ++i) {
        ysSDLAudioSource *source = static_cast<ysSDLAudioSource *>(m_audioSources.Get(i));
        source->Mix(output, sampleCount);
    }
}

ysAudioBuffer *ysSDLAudioDevice::CreateBuffer(const ysAudioParameters *parameters, SampleOffset size) {
    ysSDLAudioBuffer *newBuffer = m_audioBuffers.NewGeneric<ysSDLAudioBuffer>();
    newBuffer->Initialize(size, *parameters);

    return newBuffer;
}

ysAudioSource *ysSDLAudioDevice::CreateSource(const ysAudioParameters *parameters, SampleOffset size) {
    LockAudio();

    ysSDLAudioSource *newSource = m_audioSources.NewGeneric<ysSDLAudioSource>();
    newSource->Initialize(this, parameters, size);

    UnlockAudio();

    return newSource;
}

ysAudioSource *ysSDLAudioDevice::CreateSource(ysAudioBuffer *sourceBuffer) {
    ysAudioSource *newSource = CreateSource(sourceBuffer->GetAudioParameters(), sourceBuffer->GetSampleCount());
    newSource->SetDataBuffer(sourceBuffer);

    return newSource;
}

void ysSDLAudioDevice::UpdateAudioSources() {
    /* Sources are looping ring buffers and stop themselves in Mix(), nothing to clean up */
}

ysSDLAudioSource::ysSDLAudioSource() : ysAudioSource(API::SDL) {
    m_device = nullptr;
    m_ring = nullptr;
    m_playCursor = 0;
    m_deviceLocked = false;
}

ysSDLAudioSource::~ysSDLAudioSource() {
    /* void */
}

void ysSDLAudioSource::Initialize(ysSDLAudioDevice *device, const ysAudioParameters *parameters, SampleOffset size) {
    m_device = device;
    m_audioParameters = *parameters;
    m_bufferSize = size;
    m_bufferMode = Mode::Stop;

    m_ring = new Sint16[size]();
    m_playCursor = 0;
}

void ysSDLAudioSource::AcquireDevice() {
    if (m_deviceLocked) return;

    m_device->LockAudio();
    m_deviceLocked = true;
}

void ysSDLAudioSource::ReleaseDevice() {
    if (!m_deviceLocked) return;

    m_device->UnlockAudio();
    m_deviceLocked = false;
}

ysError ysSDLAudioSource::LockEntireBuffer(void **buffer, SampleOffset *samples) {
    AcquireDevice();

    ysError result = ysAudioSource::LockEntireBuffer(buffer, samples);
    if (result != ysError::None) {
        ReleaseDevice();
        return result;
    }

    *buffer = m_ring;

    return ysError::None;
}

ysError ysSDLAudioSource::UnlockBuffer(void *buffer, SampleOffset samples) {
    ysError result = ysAudioSource::UnlockBuffer(buffer, samples);
    ReleaseDevice();

    return result;
}

ysError ysSDLAudioSource::LockBufferSegment(SampleOffset offset, SampleOffset samples, void **segment1, SampleOffset *size1, void **segment2, SampleOffset *size2) {
    AcquireDevice();

    ysError result = ysAudioSource::LockBufferSegment(offset, samples, segment1, size1, segment2, size2);
    if (result != ysError::None) {
        ReleaseDevice();
        return result;
    }

    // The base class validated the range and sized both segments in samples.
    // Segment two starts at the beginning of the ring when the write wraps.
    *segment1 = m_ring + offset;
    *segment2 = m_ring;

    return ysError::None;
}

ysError ysSDLAudioSource::UnlockBufferSegments(void *segment1, SampleOffset segment1Size, void *segment2, SampleOffset segment2Size) {
    ysError result = ysAudioSource::UnlockBufferSegments(segment1, segment1Size, segment2, segment2Size);
    ReleaseDevice();

    return result;
}

ysError ysSDLAudioSource::SetMode(ysAudioSource::Mode mode) {
    AcquireDevice();

    ysError result = ysAudioSource::SetMode(mode);
    ReleaseDevice();

    return result;
}

ysError ysSDLAudioSource::SetDataBuffer(ysAudioBuffer *buffer) {
    YDS_ERROR_DECLARE("SetDataBuffer");

    YDS_NESTED_ERROR_CALL(ysAudioSource::SetDataBuffer(buffer));

    ysSDLAudioBuffer *sdlBuffer = static_cast<ysSDLAudioBuffer *>(buffer);
    if (sdlBuffer->GetBuffer() == nullptr) return YDS_ERROR_RETURN(ysError::None);

    AcquireDevice();

    const SampleOffset count = (buffer->GetSampleCount() < m_bufferSize)
        ? buffer->GetSampleCount()
        : m_bufferSize;
    memcpy(m_ring, sdlBuffer->GetBuffer(), sizeof(Sint16) * count);

    ReleaseDevice();

    return YDS_ERROR_RETURN(ysError::None);
}

SampleOffset ysSDLAudioSource::GetCurrentPosition() {
    AcquireDevice();
    const SampleOffset position = m_playCursor;
    ReleaseDevice();

    return position;
}

SampleOffset ysSDLAudioSource::GetCurrentWritePosition() {
    AcquireDevice();
    const SampleOffset position = (m_bufferSize > 0)
        ? (m_playCursor + WriteLeadSamples) % m_bufferSize
        : 0;
    ReleaseDevice();

    return position;
}

void ysSDLAudioSource::Seek(SampleOffset offset) {
    AcquireDevice();
    m_playCursor = (m_bufferSize > 0) ? offset % m_bufferSize : 0;
    ReleaseDevice();
}

ysError ysSDLAudioSource::Destroy() {
    YDS_ERROR_DECLARE("Destroy");

    YDS_NESTED_ERROR_CALL(ysAudioSource::Destroy());

    AcquireDevice();
    m_bufferMode = Mode::Stop;
    delete[] m_ring;
    m_ring = nullptr;
    ReleaseDevice();

    return YDS_ERROR_RETURN(ysError::None);
}

void ysSDLAudioSource::Mix(Sint16 *output, int sampleCount) {
    if (m_ring == nullptr || m_bufferSize == 0) return;
    if (m_bufferMode != Mode::Play && m_bufferMode != Mode::PlayOnce && m_bufferMode != Mode::Loop) return;

    for (int i = 0; i < sampleCount; ++i) {
        const int sample = (int)(m_ring[m_playCursor] * m_volume);
        output[i] = ClampSample(output[i] + sample);

        ++m_playCursor;
        if (m_playCursor >= m_bufferSize) {
            m_playCursor = 0;

            if (m_bufferMode != Mode::Loop) {
                m_bufferMode = Mode::Stop;
                break;
            }
        }
    }
}

ysSDLAudioBuffer::ysSDLAudioBuffer() : ysAudioBuffer(API::SDL) {
    m_buffer = nullptr;
}

ysSDLAudioBuffer::~ysSDLAudioBuffer() {
    delete[] m_buffer;
}

ysError ysSDLAudioBuffer::Initialize(SampleOffset samples, const ysAudioParameters &parameters) {
    YDS_ERROR_DECLARE("Initialize");

    YDS_NESTED_ERROR_CALL(ysAudioBuffer::Initialize(samples, parameters));

    delete[] m_buffer;
    m_buffer = new Sint16[samples]();

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysSDLAudioBuffer::EditBuffer(void *data) {
    YDS_ERROR_DECLARE("EditBuffer");

    if (m_buffer == nullptr) return YDS_ERROR_RETURN(ysError::NoAudioBuffer);

    memcpy(m_buffer, data, GetBufferSize());

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysSDLAudioBuffer::EditBufferSegment(void *data, SampleOffset samples, SampleOffset offset) {
    YDS_ERROR_DECLARE("EditBufferSegment");

    if (m_buffer == nullptr) return YDS_ERROR_RETURN(ysError::NoAudioBuffer);
    if (offset + samples > m_samples) return YDS_ERROR_RETURN(ysError::OutOfBounds);

    memcpy(m_buffer + offset, data, sizeof(Sint16) * samples);

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysSDLAudioBuffer::Destroy() {
    YDS_ERROR_DECLARE("Destroy");

    delete[] m_buffer;
    m_buffer = nullptr;

    return YDS_ERROR_RETURN(ysError::None);
}
