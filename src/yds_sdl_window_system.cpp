#include "../include/yds_sdl_window_system.h"

#include "../include/yds_sdl_window.h"
#include "../include/yds_sdl_input_system.h"

ysSDLMonitor::ysSDLMonitor() : ysMonitor(ysWindowSystemObject::Platform::SDL) {
    InitializeDeviceName(256);
}

ysSDLMonitor::~ysSDLMonitor() {
    /* void */
}

ysSDLWindowSystem::ysSDLWindowSystem() : ysWindowSystem(Platform::SDL) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    // Must be set before the first window is created. These match the
    // Windows path: OpenGL 4.3 core profile with a multisampled framebuffer.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
}

ysSDLWindowSystem::~ysSDLWindowSystem() {
    // Windows must be destroyed while the video subsystem is still running
    DeleteAllWindows();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

ysError ysSDLWindowSystem::NewWindow(ysWindow **newWindow) {
    YDS_ERROR_DECLARE("NewWindow");

    if (newWindow == nullptr) return YDS_ERROR_RETURN(ysError::InvalidParameter);

    ysSDLWindow *sdlWindow = m_windowArray.NewGeneric<ysSDLWindow>();
    *newWindow = static_cast<ysWindow *>(sdlWindow);

    return YDS_ERROR_RETURN(ysError::None);
}

ysMonitor *ysSDLWindowSystem::NewMonitor() {
    return m_monitorArray.NewGeneric<ysSDLMonitor>();
}

void ysSDLWindowSystem::SurveyMonitors() {
    ysWindowSystem::SurveyMonitors();

    const int displayCount = SDL_GetNumVideoDisplays();
    for (int i = 0; i < displayCount; ++i) {
        SDL_Rect bounds;
        SDL_DisplayMode mode;
        if (SDL_GetDisplayBounds(i, &bounds) != 0) continue;
        if (SDL_GetDesktopDisplayMode(i, &mode) != 0) continue;

        ysMonitor *newMonitor = NewMonitor();
        newMonitor->SetOrigin(bounds.x, bounds.y);
        newMonitor->SetLogicalSize(bounds.w, bounds.h);
        newMonitor->SetPhysicalSize(mode.w, mode.h);

        const char *name = SDL_GetDisplayName(i);
        newMonitor->SetDeviceName(name != nullptr ? name : "");

        newMonitor->CalculateScaling();
    }
}

void ysSDLWindowSystem::ProcessMessages() {
    ysSDLInputSystem *inputSystem = static_cast<ysSDLInputSystem *>(GetInputSystem());

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        {
            // Closing the application closes every window
            for (int i = GetWindowCount() - 1; i >= 0; --i) {
                ysWindow *window = GetWindow(i);
                window->OnCloseWindow();
                CloseWindow(window);
            }
            break;
        }
        case SDL_WINDOWEVENT:
        {
            ysWindow *target = FindWindowFromID(event.window.windowID);
            if (target == nullptr) break;

            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                target->OnCloseWindow();
                CloseWindow(target);
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                if (!target->IsResizing()) {
                    target->OnResizeWindow(event.window.data1, event.window.data2);
                }
                break;
            case SDL_WINDOWEVENT_MOVED:
                target->OnMoveWindow(event.window.data1, event.window.data2);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                target->OnActivate();
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                target->OnDeactivate();
                break;
            default:
                break;
            }
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (inputSystem != nullptr) inputSystem->ProcessKeyEvent(event.key);
            if (event.type == SDL_KEYDOWN) {
                ysWindow *target = FindWindowFromID(event.key.windowID);
                if (target != nullptr) target->OnKeyDown(event.key.keysym.sym);
            }
            break;
        case SDL_MOUSEMOTION:
            if (inputSystem != nullptr) inputSystem->ProcessMouseMotion(event.motion);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (inputSystem != nullptr) inputSystem->ProcessMouseButton(event.button);
            break;
        case SDL_MOUSEWHEEL:
            if (inputSystem != nullptr) inputSystem->ProcessMouseWheel(event.wheel);
            break;
        default:
            break;
        }
    }
}

ysWindow *ysSDLWindowSystem::FindWindowFromID(Uint32 windowID) {
    for (int i = 0; i < m_windowArray.GetNumObjects(); i++) {
        ysSDLWindow *sdlWindow = static_cast<ysSDLWindow *>(m_windowArray.Get(i));
        if (sdlWindow->GetWindowID() == windowID) return sdlWindow;
    }

    return nullptr;
}

void ysSDLWindowSystem::ConfineCursor(ysWindow *window) {
    if (IsCursorConfined()) return;
    ysWindowSystem::ConfineCursor(window);

    ysSDLWindow *sdlWindow = static_cast<ysSDLWindow *>(window);
    SDL_SetWindowGrab(sdlWindow->GetSDLWindow(), SDL_TRUE);
}

void ysSDLWindowSystem::ReleaseCursor() {
    if (!IsCursorConfined()) return;
    ysWindowSystem::ReleaseCursor();

    SDL_SetWindowGrab(SDL_GetGrabbedWindow(), SDL_FALSE);
}

void ysSDLWindowSystem::SetCursorPosition(int x, int y) {
    SDL_WarpMouseGlobal(x, y);
}

void ysSDLWindowSystem::SetCursorVisible(bool visible) {
    ysWindowSystem::SetCursorVisible(visible);
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}
