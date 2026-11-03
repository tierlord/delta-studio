#ifndef YDS_OPENGL_SDL_CONTEXT_H
#define YDS_OPENGL_SDL_CONTEXT_H

#include "yds_opengl_context.h"

#include <SDL.h>

class ysOpenGLDevice;

class ysOpenGLSDLContext : public ysOpenGLVirtualContext {
    friend ysOpenGLDevice;

public:
    ysOpenGLSDLContext();
    virtual ~ysOpenGLSDLContext();

    ysError CreateRenderingContext(ysOpenGLDevice *device, ysWindow *window, int major, int minor);

    virtual ysError DestroyContext();
    virtual ysError TransferContext(ysOpenGLVirtualContext *context);
    virtual ysError SetContextMode(ContextMode mode);
    virtual ysError SetContext(ysRenderingContext *realContext);
    virtual ysError Present();

protected:
    SDL_Window *m_sdlWindow;
    SDL_GLContext m_contextHandle;

    ysOpenGLDevice *m_device;

protected:
    void LoadAllExtensions();
};

#endif /* YDS_OPENGL_SDL_CONTEXT_H */
