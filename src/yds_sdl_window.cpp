#include "../include/yds_sdl_window.h"

#include "../include/yds_monitor.h"

namespace {
    Uint32 GetSDLWindowFlags(ysWindow::WindowStyle style) {
        Uint32 flags = SDL_WINDOW_OPENGL;

        switch (style) {
        case ysWindow::WindowStyle::Windowed:
            flags |= SDL_WINDOW_RESIZABLE;
            break;
        case ysWindow::WindowStyle::Fullscreen:
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        case ysWindow::WindowStyle::Popup:
            flags |= SDL_WINDOW_BORDERLESS;
            break;
        default:
            break;
        }

        return flags;
    }
}

ysSDLWindow::ysSDLWindow() : ysWindow(Platform::SDL) {
    m_window = nullptr;
    m_windowID = 0;
}

ysSDLWindow::~ysSDLWindow() {
    if (m_window != nullptr) SDL_DestroyWindow(m_window);
}

ysError ysSDLWindow::InitializeWindow(ysWindow *parent, const char *title, WindowStyle style, int x, int y, int width, int height, ysMonitor *monitor) {
    YDS_ERROR_DECLARE("InitializeWindow");

    if (!CheckCompatibility(parent)) return YDS_ERROR_RETURN(ysError::IncompatiblePlatforms);

    YDS_NESTED_ERROR_CALL(ysWindow::InitializeWindow(parent, title, style, x, y, width, height, monitor));

    const Uint32 flags = GetSDLWindowFlags(style);
    m_window = SDL_CreateWindow(title, x, y, width, height, flags);

    // Not every GPU/driver provides the multisampled pixel format that is requested
    // when the window system is set up. Retry without multisampling before giving up.
    if (m_window == nullptr) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        m_window = SDL_CreateWindow(title, x, y, width, height, flags);
    }

    if (m_window == nullptr) return YDS_ERROR_RETURN(ysError::CouldNotCreateWindow);

    m_windowID = SDL_GetWindowID(m_window);

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysSDLWindow::InitializeWindow(ysWindow *parent, const char *title, WindowStyle style, ysMonitor *monitor) {
    YDS_ERROR_DECLARE("InitializeWindow");

    YDS_NESTED_ERROR_CALL(InitializeWindow(
        parent, title, style,
        monitor->GetOriginX(), monitor->GetOriginY(),
        monitor->GetPhysicalWidth(), monitor->GetPhysicalHeight(), monitor));

    return YDS_ERROR_RETURN(ysError::None);
}

bool ysSDLWindow::SetWindowStyle(WindowStyle style) {
    if (m_window == nullptr) return false;
    if (!ysWindow::SetWindowStyle(style)) return false;

    if (style == WindowStyle::Windowed) {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        const int x = GetPositionX();
        const int y = GetPositionY();

        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowBordered(m_window, SDL_TRUE);
        SetLocation(x, y);
        SetWindowSize(width, height);
        SDL_ShowWindow(m_window);
        SDL_RaiseWindow(m_window);
    }
    else if (style == WindowStyle::Fullscreen) {
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowWindow(m_window);
        SDL_RaiseWindow(m_window);
    }
    else if (style == WindowStyle::Popup) {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        const int x = GetPositionX();
        const int y = GetPositionY();

        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowBordered(m_window, SDL_FALSE);
        SetLocation(x, y);
        SetWindowSize(width, height);
        SDL_ShowWindow(m_window);
        SDL_RaiseWindow(m_window);
    }

    return true;
}

void ysSDLWindow::ScreenToLocal(int &x, int &y) const {
    if (m_window == nullptr) {
        x = y = 0;
        return;
    }

    int windowX, windowY;
    SDL_GetWindowPosition(m_window, &windowX, &windowY);

    x -= windowX;
    y = GetScreenHeight() - (y - windowY);
}

bool ysSDLWindow::IsVisible() {
    if (m_window == nullptr) return false;

    const Uint32 flags = SDL_GetWindowFlags(m_window);
    if (flags & SDL_WINDOW_HIDDEN) return false;
    if (flags & SDL_WINDOW_MINIMIZED) return false;

    return true;
}

int ysSDLWindow::GetScreenWidth() const {
    if (m_window == nullptr) return 0;

    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);

    return width;
}

int ysSDLWindow::GetScreenHeight() const {
    if (m_window == nullptr) return 0;

    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);

    return height;
}

// Abstraction Layer

void ysSDLWindow::Close() {
    ysWindow::Close();

    if (m_window != nullptr) SDL_DestroyWindow(m_window);
    m_window = nullptr;
    m_windowID = 0;
}

void ysSDLWindow::SetTitle(const char *title) {
    ysWindow::SetTitle(title);

    if (m_window != nullptr) SDL_SetWindowTitle(m_window, title);
}

void ysSDLWindow::SetState(WindowState state) {
    if (m_window == nullptr) return;

    ysWindow::SetState(state);

    switch (state) {
    case WindowState::Visible:
        SDL_ShowWindow(m_window);
        break;
    case WindowState::Hidden:
        SDL_HideWindow(m_window);
        break;
    case WindowState::Maximized:
        SDL_MaximizeWindow(m_window);
        break;
    case WindowState::Minimized:
        SDL_MinimizeWindow(m_window);
        break;
    default:
        break;
    }

    SDL_RaiseWindow(m_window);
}

void ysSDLWindow::AL_SetSize(int width, int height) {
    if (m_window == nullptr) return;

    ysWindow::AL_SetSize(width, height);
    SDL_SetWindowSize(m_window, width, height);
}

void ysSDLWindow::AL_SetLocation(int x, int y) {
    if (m_window == nullptr) return;

    ysWindow::AL_SetLocation(x, y);
    SDL_SetWindowPosition(m_window, x, y);
}
