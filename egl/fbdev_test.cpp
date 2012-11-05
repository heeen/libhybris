#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdio.h>

#include "fbdev_window.h"
#include "offscreen_window.h"

class EGLClient {
public:
    EGLClient() {
        EGLConfig ecfg;
        EGLint num_config;
        EGLint attr[] = {       // some attributes to set up our egl-interface
            EGL_BUFFER_SIZE, 32,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_NONE
        };
        EGLint ctxattr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            printf("ERROR: Could not get default display\n");
            return;
        }

        printf("INFO: Successfully retrieved default display!\n");

        eglInitialize(display, 0, 0);
        eglChooseConfig((EGLDisplay) display, attr, &ecfg, 1, &num_config);

        printf("INFO: Initialized display with default configuration\n");

        window = new OffscreenNativeWindow(720, 1280);
        printf("INFO: Created native window %p\n", window);
        printf("creating window surface...\n");
        surface = eglCreateWindowSurface((EGLDisplay) display, ecfg, *window, NULL);
        assert(surface != EGL_NO_SURFACE);
        printf("INFO: Created our main window surface %p\n", surface);
        context = eglCreateContext((EGLDisplay) display, ecfg, EGL_NO_CONTEXT, ctxattr);
        assert(surface != EGL_NO_CONTEXT);
        printf("INFO: Created context for display\n");
        frame=0;
    };
    void render() {
            assert(eglMakeCurrent((EGLDisplay) display, surface, surface, context) == EGL_TRUE);
            printf("INFO: Made context and surface current for display\n");
            glViewport ( 0 , 0 , 1280, 720);
            printf("client frame %i\n", frame++);
            glClearColor ( 1.00 , (frame & 1) * 1.0f , ((float)(frame % 255))/255.0f, 1.);    // background color
            glClear(GL_COLOR_BUFFER_BIT);
            eglSwapBuffers(display, surface);
            printf("client swapped\n");
    }
    ANativeWindowBuffer* getNativeBuffer() {
        return 0; // fixme
    }
    int frame;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    OffscreenNativeWindow* window;
};

class EGLCompositor {
public:
    EGLCompositor() {
        EGLConfig ecfg;
        EGLint num_config;
        EGLint attr[] = {       // some attributes to set up our egl-interface
            EGL_BUFFER_SIZE, 32,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_NONE
        };
        EGLint ctxattr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            printf("ERROR: Could not get default display\n");
            return;
        }

        printf("INFO: Successfully retrieved default display!\n");

        eglInitialize(display, 0, 0);
        eglChooseConfig((EGLDisplay) display, attr, &ecfg, 1, &num_config);

        printf("INFO: Initialized display with default configuration\n");

        window = new FbDevNativeWindow();
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
        frame=0;
    };
    void render(EGLClient& client) {
            assert(eglMakeCurrent((EGLDisplay) display, surface, surface, context) == EGL_TRUE);
            printf("INFO: Made context and surface current for display\n");


            EGLClientBuffer cbuf = (EGLClientBuffer) client.getNativeBuffer();
            EGLint attrs[] = {
                EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
                EGL_NONE,
            };
            EGLImageKHR image = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, cbuf, attrs);
            if (image == EGL_NO_IMAGE_KHR) {
                EGLint error = eglGetError();
                printf("error creating EGLImage: %#x", error);
            }
            eglDestroyImageKHR(image);


            glViewport ( 0 , 0 , 1280, 720);
            printf("compositor frame %i\n", frame++);
            glClearColor ( (frame & 1) * 1.0f , 0.06 , ((float)(frame % 255))/255.0f, 1.);    // background color
            glClear(GL_COLOR_BUFFER_BIT);
            eglSwapBuffers(display, surface);
            printf("compositor swapped\n");
    }
    int frame;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    FbDevNativeWindow* window;
    GLuint texture;
    EGLImageKHR image;
};

int main(int argc, char **argv)
{
    EGLCompositor compositor;
    EGLClient client;
    while(1) {
        client.render();
        compositor.render(client);
    }
}
