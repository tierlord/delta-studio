#ifndef YDS_SDL_INPUT_SYSTEM_H
#define YDS_SDL_INPUT_SYSTEM_H

#include "yds_input_system.h"

#include <SDL.h>

class ysSDLInputDevice : public ysInputDevice {
public:
    ysSDLInputDevice();
    ~ysSDLInputDevice();
};

class ysSDLInputSystem : public ysInputSystem {
public:
    ysSDLInputSystem();
    virtual ~ysSDLInputSystem();

    // Event handlers. These are fed by ysSDLWindowSystem::ProcessMessages()
    void ProcessKeyEvent(const SDL_KeyboardEvent &event);
    void ProcessMouseMotion(const SDL_MouseMotionEvent &event);
    void ProcessMouseButton(const SDL_MouseButtonEvent &event);
    void ProcessMouseWheel(const SDL_MouseWheelEvent &event);

protected:
    virtual ysError CreateDevices();
    virtual ysInputDevice *CreateDevice(ysInputDevice::InputDeviceType type, int id);

protected:
    ysInputDevice *m_keyboardDevice;
    ysInputDevice *m_mouseDevice;
};

#endif /* YDS_SDL_INPUT_SYSTEM_H */
