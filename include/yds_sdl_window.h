#ifndef YDS_SDL_WINDOW_H
#define YDS_SDL_WINDOW_H

#include "yds_window.h"

#include <SDL.h>

class ysSDLWindowSystem;

class ysSDLWindow : public ysWindow {
    friend ysSDLWindowSystem;

public:
    ysSDLWindow();
    virtual ~ysSDLWindow();

    // Overrides

    virtual ysError InitializeWindow(ysWindow *parent, const char *title, WindowStyle style, int x, int y, int width, int height, ysMonitor *monitor);
    virtual ysError InitializeWindow(ysWindow *parent, const char *title, WindowStyle style, ysMonitor *monitor);

    virtual bool SetWindowStyle(WindowStyle style);

    virtual void ScreenToLocal(int &x, int &y) const;

    virtual bool IsVisible();

    virtual int GetScreenWidth() const;
    virtual int GetScreenHeight() const;

public:
    // SDL Specific

    SDL_Window *GetSDLWindow() const { return m_window; }
    Uint32 GetWindowID() const { return m_windowID; }

protected:
    // Abstraction Layer

    virtual void Close();
    virtual void SetTitle(const char *title);
    virtual void SetState(WindowState state = WindowState::Visible);

    virtual void AL_SetSize(int width, int height);
    virtual void AL_SetLocation(int x, int y);

protected:
    SDL_Window *m_window;
    Uint32 m_windowID;
};

#endif /* YDS_SDL_WINDOW_H */
