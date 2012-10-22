#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdio.h>

#include "fbdev_window.h"
#include "offscreen_window.h"
int main(int argc, char **argv)
{
    EGLDisplay display;
    EGLConfig ecfg;
    EGLint num_config;
    EGLint attr[] = {       // some attributes to set up our egl-interface
        EGL_BUFFER_SIZE, 32,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLSurface surface;
    EGLint ctxattr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context;
    printf("\n\n========================== about to get display...\n");
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("ERROR: Could not get default display\n");
        return -1;
    }

    printf("INFO: Successfully retrieved default display!\n");

    eglInitialize(display, 0, 0);
    eglChooseConfig((EGLDisplay) display, attr, &ecfg, 1, &num_config);

    printf("INFO: Initialized display with default configuration\n");

    printf("creating native window...\n");
//    FbDevNativeWindow *window = new FbDevNativeWindow();
    OffscreenNativeWindow *window = new OffscreenNativeWindow(720, 1280);
    printf("INFO: Created native window %p\n", window);

    printf("creating window surface...\n");
    surface = eglCreateWindowSurface((EGLDisplay) display, ecfg, *window, NULL);
    assert(surface != EGL_NO_SURFACE);

    printf("INFO: Created our main window surface %p\n", surface);

    context = eglCreateContext((EGLDisplay) display, ecfg, EGL_NO_CONTEXT, ctxattr);
    assert(surface != EGL_NO_CONTEXT);

    printf("INFO: Created context for display\n");

    assert(eglMakeCurrent((EGLDisplay) display, surface, surface, context) == EGL_TRUE);

    printf("INFO: Made context and surface current for display\n");

    glViewport ( 0 , 0 , 1280, 720);
    int frame=0;
    while (1) {
        printf("frame %i\n", frame++);
        glClearColor ( 1.00 , 0.06 , ((float)(frame % 255))/255.0f, 1.);    // background color
        printf("set clear color\n");
        glClear(GL_COLOR_BUFFER_BIT);
        printf("cleared\n");
        eglSwapBuffers(display, surface);
        printf("swapped\n");
    }

    printf("stop\n");

#if 0
(*egldestroycontext)((EGLDisplay) display, context);
    printf("destroyed context\n");

    (*egldestroysurface)((EGLDisplay) display, surface);
    printf("destroyed surface\n");
    (*eglterminate)((EGLDisplay) display);
    printf("terminated\n");
    android_dlclose(baz);
#endif
}
