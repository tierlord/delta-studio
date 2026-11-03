#include "../include/yds_opengl_sdl_context.h"

#include "../include/yds_opengl_device.h"
#include "../include/yds_sdl_window.h"

// Loads an OpenGL entry point into the context member of the same name.
// The member type already describes the signature, so no casts are needed.
#define YDS_LOAD_GL_PROC(name) name = (decltype(name))SDL_GL_GetProcAddress(#name)

ysOpenGLSDLContext::ysOpenGLSDLContext() : ysOpenGLVirtualContext(ysWindowSystemObject::Platform::SDL) {
    m_sdlWindow = nullptr;
    m_contextHandle = nullptr;
    m_device = nullptr;
}

ysOpenGLSDLContext::~ysOpenGLSDLContext() {
    /* void */
}

ysError ysOpenGLSDLContext::CreateRenderingContext(ysOpenGLDevice *device, ysWindow *window, int major, int minor) {
    YDS_ERROR_DECLARE("CreateRenderingContext");

    if (window->GetPlatform() != ysWindowSystemObject::Platform::SDL) {
        return YDS_ERROR_RETURN(ysError::IncompatiblePlatforms);
    }

    ysSDLWindow *sdlWindow = static_cast<ysSDLWindow *>(window);
    SDL_Window *windowHandle = sdlWindow->GetSDLWindow();
    if (windowHandle == nullptr) return YDS_ERROR_RETURN(ysError::InvalidParameter);

    // Default
    m_device = nullptr;
    m_sdlWindow = nullptr;
    m_targetWindow = nullptr;
    m_contextHandle = nullptr;
    m_isRealContext = false;

    // Create a context if one doesn't already exist. Additional windows
    // share the device's context, which SetContext() activates for them.
    if (device->UpdateContext() == nullptr) {
        SDL_GLContext newContext = SDL_GL_CreateContext(windowHandle);
        if (newContext == nullptr) return YDS_ERROR_RETURN(ysError::CouldNotCreateContext);

        if (SDL_GL_MakeCurrent(windowHandle, newContext) != 0) {
            SDL_GL_DeleteContext(newContext);
            return YDS_ERROR_RETURN(ysError::CouldNotActivateContext);
        }

        m_contextHandle = newContext;
        m_isRealContext = true;

        device->UpdateContext();
    }

    m_device = device;
    m_sdlWindow = windowHandle;
    m_targetWindow = window;

    LoadAllExtensions();

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysOpenGLSDLContext::DestroyContext() {
    YDS_ERROR_DECLARE("DestroyContext");

    SDL_GL_MakeCurrent(nullptr, nullptr);
    if (m_contextHandle) {
        SDL_GL_DeleteContext(m_contextHandle);
        m_contextHandle = nullptr;
    }

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysOpenGLSDLContext::TransferContext(ysOpenGLVirtualContext *context) {
    YDS_ERROR_DECLARE("TransferContext");

    if (context->GetPlatform() != ysWindowSystemObject::Platform::SDL) return YDS_ERROR_RETURN(ysError::IncompatiblePlatforms);

    ysOpenGLSDLContext *sdlContext = static_cast<ysOpenGLSDLContext *>(context);
    sdlContext->m_contextHandle = m_contextHandle;
    sdlContext->m_isRealContext = true;

    m_isRealContext = false;
    m_contextHandle = nullptr;

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysOpenGLSDLContext::SetContextMode(ContextMode mode) {
    YDS_ERROR_DECLARE("SetContextMode");

    ysWindow *window = GetWindow();

    if (mode == ysRenderingContext::ContextMode::Fullscreen) {
        window->SetWindowStyle(ysWindow::WindowStyle::Fullscreen);
    }
    else if (mode == ysRenderingContext::ContextMode::Windowed) {
        window->SetWindowStyle(ysWindow::WindowStyle::Windowed);
    }

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysOpenGLSDLContext::SetContext(ysRenderingContext *realContext) {
    YDS_ERROR_DECLARE("SetContext");

    if (realContext != nullptr) {
        ysOpenGLSDLContext *realOpenglContext = static_cast<ysOpenGLSDLContext *>(realContext);
        if (SDL_GL_MakeCurrent(m_sdlWindow, realOpenglContext->m_contextHandle) != 0) {
            return YDS_ERROR_RETURN(ysError::CouldNotActivateContext);
        }
    }
    else {
        SDL_GL_MakeCurrent(nullptr, nullptr);
    }

    return YDS_ERROR_RETURN(ysError::None);
}

ysError ysOpenGLSDLContext::Present() {
    YDS_ERROR_DECLARE("Present");

    if (m_sdlWindow == nullptr) return YDS_ERROR_RETURN(ysError::NoContext);

    SDL_GL_SwapWindow(m_sdlWindow);

    return YDS_ERROR_RETURN(ysError::None);
}

void ysOpenGLSDLContext::LoadAllExtensions() {
    YDS_LOAD_GL_PROC(glGenBuffers);
    YDS_LOAD_GL_PROC(glDeleteBuffers);
    YDS_LOAD_GL_PROC(glBindBuffer);
    YDS_LOAD_GL_PROC(glBindBufferRange);
    YDS_LOAD_GL_PROC(glBufferData);
    YDS_LOAD_GL_PROC(glGenVertexArrays);
    YDS_LOAD_GL_PROC(glDeleteVertexArrays);
    YDS_LOAD_GL_PROC(glBindVertexArray);
    YDS_LOAD_GL_PROC(glEnableVertexAttribArray);
    YDS_LOAD_GL_PROC(glVertexAttribPointer);
    YDS_LOAD_GL_PROC(glVertexAttribIPointer);
    YDS_LOAD_GL_PROC(glVertexAttrib3f);
    YDS_LOAD_GL_PROC(glVertexAttrib4f);

    // Shaders
    YDS_LOAD_GL_PROC(glDeleteShader);
    YDS_LOAD_GL_PROC(glDeleteProgram);

    YDS_LOAD_GL_PROC(glCreateShader);
    YDS_LOAD_GL_PROC(glShaderSource);
    YDS_LOAD_GL_PROC(glCompileShader);
    YDS_LOAD_GL_PROC(glCreateProgram);
    YDS_LOAD_GL_PROC(glAttachShader);
    YDS_LOAD_GL_PROC(glDetachShader);
    YDS_LOAD_GL_PROC(glLinkProgram);
    YDS_LOAD_GL_PROC(glUseProgram);
    YDS_LOAD_GL_PROC(glBindAttribLocation);
    YDS_LOAD_GL_PROC(glBindFragDataLocation);
    YDS_LOAD_GL_PROC(glGetFragDataLocation);
    YDS_LOAD_GL_PROC(glGetUniformLocation);
    YDS_LOAD_GL_PROC(glGetShaderiv);
    YDS_LOAD_GL_PROC(glGetShaderInfoLog);
    YDS_LOAD_GL_PROC(glDrawBuffers);

    YDS_LOAD_GL_PROC(glUniform4fv);
    YDS_LOAD_GL_PROC(glUniform3fv);
    YDS_LOAD_GL_PROC(glUniform2fv);
    YDS_LOAD_GL_PROC(glUniform4f);
    YDS_LOAD_GL_PROC(glUniform3f);
    YDS_LOAD_GL_PROC(glUniform2f);
    YDS_LOAD_GL_PROC(glUniform1f);
    YDS_LOAD_GL_PROC(glUniform1i);

    YDS_LOAD_GL_PROC(glUniformMatrix4fv);
    YDS_LOAD_GL_PROC(glUniformMatrix3fv);

    YDS_LOAD_GL_PROC(glGetProgramiv);

    YDS_LOAD_GL_PROC(glGetActiveUniformName);
    YDS_LOAD_GL_PROC(glGetActiveUniformsiv);
    YDS_LOAD_GL_PROC(glGetActiveUniform);

    YDS_LOAD_GL_PROC(glMapBuffer);
    YDS_LOAD_GL_PROC(glMapBufferRange);
    YDS_LOAD_GL_PROC(glUnmapBuffer);

    YDS_LOAD_GL_PROC(glDrawElementsBaseVertex);

    // Textures
    YDS_LOAD_GL_PROC(glTexImage2DMultisample);
    YDS_LOAD_GL_PROC(glActiveTexture);
    YDS_LOAD_GL_PROC(glGenerateMipmap);

    // Buffers
    YDS_LOAD_GL_PROC(glGenRenderbuffers);
    YDS_LOAD_GL_PROC(glDeleteRenderbuffers);
    YDS_LOAD_GL_PROC(glBindRenderbuffer);
    YDS_LOAD_GL_PROC(glRenderbufferStorage);
    YDS_LOAD_GL_PROC(glRenderbufferStorageMultisample);
    YDS_LOAD_GL_PROC(glCopyBufferSubData);
    YDS_LOAD_GL_PROC(glBufferSubData);

    YDS_LOAD_GL_PROC(glGenFramebuffers);
    YDS_LOAD_GL_PROC(glDeleteFramebuffers);
    YDS_LOAD_GL_PROC(glBindFramebuffer);
    YDS_LOAD_GL_PROC(glFramebufferTexture2D);
    YDS_LOAD_GL_PROC(glFramebufferRenderbuffer);
    YDS_LOAD_GL_PROC(glCheckFramebufferStatus);

    YDS_LOAD_GL_PROC(glBlitFramebuffer);

    // Blending
    YDS_LOAD_GL_PROC(glBlendEquation);
}

#undef YDS_LOAD_GL_PROC
