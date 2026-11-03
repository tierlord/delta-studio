#include "../include/yds_sdl_input_system.h"

#include <stdio.h>

namespace {
    ysKey::Code TranslateKeycode(SDL_Keycode sym) {
        if (sym >= SDLK_a && sym <= SDLK_z) return (ysKey::Code)((int)ysKey::Code::A + (sym - SDLK_a));
        if (sym >= SDLK_0 && sym <= SDLK_9) return (ysKey::Code)((int)ysKey::Code::N0 + (sym - SDLK_0));
        if (sym >= SDLK_F1 && sym <= SDLK_F12) return (ysKey::Code)((int)ysKey::Code::F1 + (sym - SDLK_F1));
        if (sym >= SDLK_F13 && sym <= SDLK_F24) return (ysKey::Code)((int)ysKey::Code::F13 + (sym - SDLK_F13));

        switch (sym) {
        case SDLK_BACKSPACE: return ysKey::Code::Back;
        case SDLK_TAB: return ysKey::Code::Tab;
        case SDLK_CLEAR: return ysKey::Code::Clear;
        case SDLK_RETURN:
        case SDLK_RETURN2:
        case SDLK_KP_ENTER: return ysKey::Code::Return;
        case SDLK_LSHIFT: return ysKey::Code::LeftShift;
        case SDLK_RSHIFT: return ysKey::Code::RightShift;
        case SDLK_LCTRL: return ysKey::Code::LeftControl;
        case SDLK_RCTRL: return ysKey::Code::RightControl;
        case SDLK_LALT: return ysKey::Code::LeftMenu;
        case SDLK_RALT: return ysKey::Code::RightMenu;
        case SDLK_LGUI: return ysKey::Code::LeftWin;
        case SDLK_RGUI: return ysKey::Code::RightWin;
        case SDLK_APPLICATION: return ysKey::Code::Apps;
        case SDLK_PAUSE: return ysKey::Code::Pause;
        case SDLK_CAPSLOCK: return ysKey::Code::CapsLock;
        case SDLK_ESCAPE: return ysKey::Code::Escape;
        case SDLK_SPACE: return ysKey::Code::Space;
        case SDLK_PAGEUP: return ysKey::Code::PageUp;
        case SDLK_PAGEDOWN: return ysKey::Code::PageDown;
        case SDLK_END: return ysKey::Code::End;
        case SDLK_HOME: return ysKey::Code::Home;
        case SDLK_LEFT: return ysKey::Code::Left;
        case SDLK_UP: return ysKey::Code::Up;
        case SDLK_RIGHT: return ysKey::Code::Right;
        case SDLK_DOWN: return ysKey::Code::Down;
        case SDLK_SELECT: return ysKey::Code::Select;
        case SDLK_PRINTSCREEN: return ysKey::Code::PrintScreen;
        case SDLK_INSERT: return ysKey::Code::Insert;
        case SDLK_DELETE: return ysKey::Code::Delete;
        case SDLK_HELP: return ysKey::Code::Help;
        case SDLK_SLEEP: return ysKey::Code::Sleep;
        case SDLK_NUMLOCKCLEAR: return ysKey::Code::NumLock;
        case SDLK_SCROLLLOCK: return ysKey::Code::ScrollLock;
        case SDLK_KP_0: return ysKey::Code::Numpad0;
        case SDLK_KP_1: return ysKey::Code::Numpad1;
        case SDLK_KP_2: return ysKey::Code::Numpad2;
        case SDLK_KP_3: return ysKey::Code::Numpad3;
        case SDLK_KP_4: return ysKey::Code::Numpad4;
        case SDLK_KP_5: return ysKey::Code::Numpad5;
        case SDLK_KP_6: return ysKey::Code::Numpad6;
        case SDLK_KP_7: return ysKey::Code::Numpad7;
        case SDLK_KP_8: return ysKey::Code::Numpad8;
        case SDLK_KP_9: return ysKey::Code::Numpad9;
        case SDLK_KP_MULTIPLY: return ysKey::Code::Multiply;
        case SDLK_KP_PLUS: return ysKey::Code::Add;
        case SDLK_KP_MINUS: return ysKey::Code::Subtract;
        case SDLK_KP_PERIOD: return ysKey::Code::Decimal;
        case SDLK_KP_DIVIDE: return ysKey::Code::Divide;
        case SDLK_KP_COMMA: return ysKey::Code::Separator;
        case SDLK_COMMA: return ysKey::Code::OEM_Comma;
        case SDLK_PERIOD: return ysKey::Code::OEM_Period;
        case SDLK_MINUS: return ysKey::Code::OEM_Minus;
        case SDLK_EQUALS: return ysKey::Code::OEM_Plus;
        case SDLK_SEMICOLON: return ysKey::Code::OEM_1;
        case SDLK_SLASH: return ysKey::Code::OEM_2;
        case SDLK_BACKQUOTE: return ysKey::Code::OEM_3;
        case SDLK_LEFTBRACKET: return ysKey::Code::OEM_4;
        case SDLK_BACKSLASH: return ysKey::Code::OEM_5;
        case SDLK_RIGHTBRACKET: return ysKey::Code::OEM_6;
        case SDLK_QUOTE: return ysKey::Code::OEM_7;
        case SDLK_MUTE: return ysKey::Code::VolumeMute;
        case SDLK_VOLUMEDOWN: return ysKey::Code::VolumeDown;
        case SDLK_VOLUMEUP: return ysKey::Code::VolumeUp;
        case SDLK_AUDIONEXT: return ysKey::Code::MediaNextTrack;
        case SDLK_AUDIOPREV: return ysKey::Code::MediaPrevTrack;
        case SDLK_AUDIOSTOP: return ysKey::Code::MediaStop;
        case SDLK_AUDIOPLAY: return ysKey::Code::MediaPlay;
        default: return ysKey::Code::Undefined;
        }
    }

    // Left and right modifiers also set the generic modifier, like the Windows key map does
    ysKey::Code GetGenericModifier(ysKey::Code key) {
        switch (key) {
        case ysKey::Code::LeftShift:
        case ysKey::Code::RightShift:
            return ysKey::Code::Shift;
        case ysKey::Code::LeftControl:
        case ysKey::Code::RightControl:
            return ysKey::Code::Control;
        case ysKey::Code::LeftMenu:
        case ysKey::Code::RightMenu:
            return ysKey::Code::Menu;
        default:
            return ysKey::Code::Undefined;
        }
    }

    ysKey::Variation GetVariation(ysKey::Code key) {
        switch (key) {
        case ysKey::Code::LeftShift:
        case ysKey::Code::LeftControl:
        case ysKey::Code::LeftMenu:
            return ysKey::Variation::Left;
        case ysKey::Code::RightShift:
        case ysKey::Code::RightControl:
        case ysKey::Code::RightMenu:
            return ysKey::Variation::Right;
        default:
            return ysKey::Variation::Undefined;
        }
    }
}

ysSDLInputDevice::ysSDLInputDevice() : ysInputDevice(Platform::SDL, InputDeviceType::UNKNOWN) {
    /* void */
}

ysSDLInputDevice::~ysSDLInputDevice() {
    /* void */
}

ysSDLInputSystem::ysSDLInputSystem() : ysInputSystem(Platform::SDL) {
    m_keyboardDevice = nullptr;
    m_mouseDevice = nullptr;
}

ysSDLInputSystem::~ysSDLInputSystem() {
    /* void */
}

ysError ysSDLInputSystem::CreateDevices() {
    YDS_ERROR_DECLARE("CreateDevices");

    m_keyboardDevice = CreateDevice(ysInputDevice::InputDeviceType::KEYBOARD, 0);
    m_mouseDevice = CreateDevice(ysInputDevice::InputDeviceType::MOUSE, 0);

    return YDS_ERROR_RETURN(ysError::None);
}

ysInputDevice *ysSDLInputSystem::CreateDevice(ysInputDevice::InputDeviceType type, int id) {
    const char *baseName;
    switch (type) {
    case ysInputDevice::InputDeviceType::CUSTOM:
        baseName = "HID";
        break;
    case ysInputDevice::InputDeviceType::KEYBOARD:
        baseName = "KEYBOARD";
        break;
    case ysInputDevice::InputDeviceType::MOUSE:
        baseName = "MOUSE";
        break;
    default:
        baseName = "";
    }

    const int deviceID = (id >= 0) ? id : GetNextDeviceID(type);

    char deviceName[ysInputDevice::MAX_NAME_LENGTH];
    snprintf(deviceName, ysInputDevice::MAX_NAME_LENGTH, "%s%0.3d", baseName, deviceID);

    ysSDLInputDevice *newDevice = m_inputDeviceArray.NewGeneric<ysSDLInputDevice>();
    newDevice->SetName(deviceName);
    newDevice->SetType(type);
    newDevice->SetDeviceID(deviceID);
    newDevice->SetGeneric(false);
    newDevice->SetConnected(true);

    RegisterDevice(newDevice);

    return newDevice;
}

void ysSDLInputSystem::ProcessKeyEvent(const SDL_KeyboardEvent &event) {
    if (m_keyboardDevice == nullptr) return;

    const ysKey::Code key = TranslateKeycode(event.keysym.sym);
    if (key == ysKey::Code::Undefined) return;

    const ysKey::State state = (event.type == SDL_KEYDOWN)
        ? ysKey::State::DownTransition
        : ysKey::State::UpTransition;

    ysKeyboard *keyboard = m_keyboardDevice->GetAsKeyboard();
    keyboard->SetKeyState(key, state, GetVariation(key));

    const ysKey::Code genericModifier = GetGenericModifier(key);
    if (genericModifier != ysKey::Code::Undefined) {
        keyboard->SetKeyState(genericModifier, state, ysKey::Variation::Undefined);
    }
}

void ysSDLInputSystem::ProcessMouseMotion(const SDL_MouseMotionEvent &event) {
    if (m_mouseDevice == nullptr) return;

    ysMouse *mouse = m_mouseDevice->GetAsMouse();

    // Screen coordinates, the same space GetCursorPos() reports on Windows
    int globalX, globalY;
    SDL_GetGlobalMouseState(&globalX, &globalY);
    mouse->SetOsPosition(globalX, globalY);

    mouse->UpdatePosition(event.xrel, event.yrel, true);
}

void ysSDLInputSystem::ProcessMouseButton(const SDL_MouseButtonEvent &event) {
    if (m_mouseDevice == nullptr) return;

    ysMouse::Button button;
    switch (event.button) {
    case SDL_BUTTON_LEFT:
        button = ysMouse::Button::Left;
        break;
    case SDL_BUTTON_MIDDLE:
        button = ysMouse::Button::Middle;
        break;
    case SDL_BUTTON_RIGHT:
        button = ysMouse::Button::Right;
        break;
    case SDL_BUTTON_X1:
        button = ysMouse::Button::Aux_4;
        break;
    case SDL_BUTTON_X2:
        button = ysMouse::Button::Aux_5;
        break;
    default:
        return;
    }

    const ysMouse::ButtonState state = (event.type == SDL_MOUSEBUTTONDOWN)
        ? ysMouse::ButtonState::DownTransition
        : ysMouse::ButtonState::UpTransition;

    m_mouseDevice->GetAsMouse()->UpdateButton(button, state);
}

void ysSDLInputSystem::ProcessMouseWheel(const SDL_MouseWheelEvent &event) {
    if (m_mouseDevice == nullptr) return;

    int notches = event.y;
    if (event.direction == SDL_MOUSEWHEEL_FLIPPED) notches = -notches;

    // Windows reports 120 units per wheel notch (WHEEL_DELTA), keep the same scale
    m_mouseDevice->GetAsMouse()->UpdateWheel(notches * 120);
}
