#ifndef YDS_SDL_WINDOW_SYSTEM_H
#define YDS_SDL_WINDOW_SYSTEM_H

#include "yds_window_system.h"

#include <SDL.h>

class ysSDLMonitor : public ysMonitor {
public:
    ysSDLMonitor();
    ~ysSDLMonitor();
};

class ysSDLWindowSystem : public ysWindowSystem {
public:
    ysSDLWindowSystem();
    ~ysSDLWindowSystem();

    virtual ysError NewWindow(ysWindow **newWindow);

    virtual ysMonitor *NewMonitor();
    virtual void SurveyMonitors();

    virtual void ProcessMessages();

    ysWindow *FindWindowFromID(Uint32 windowID);

    virtual void ConfineCursor(ysWindow *window);
    virtual void ReleaseCursor();

    virtual void SetCursorPosition(int x, int y);
    virtual void SetCursorVisible(bool visible);
};

#endif /* YDS_SDL_WINDOW_SYSTEM_H */
