#include "offscreen_window.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/matroxfb.h> // for FBIO_WAITFORVSYNC
#include <sys/mman.h> //mmap, munmap
#include <errno.h>



OffscreenNativeWindow::OffscreenNativeWindow(unsigned int aWidth, unsigned int aHeight, unsigned int aFormat)
    : m_width(aWidth)
    , m_height(aHeight)
    , m_defaultWidth(aWidth)
    , m_defaultHeight(aHeight)
    , m_format(aFormat)
{
	hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t**)&m_gralloc);
    m_usage=GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_2D | GRALLOC_USAGE_SW_READ_RARELY;
    int err = gralloc_open((hw_module_t*)m_gralloc, &m_alloc);
    printf("got alloc %p err:%s\n", m_alloc, strerror(-err));

    for(unsigned int i = 0; i < NUM_BUFFERS; i++) {
        m_buffers[i] = 0;
    }
    m_frontbuffer = 0;
    m_tailbuffer = 1;
    

}

OffscreenNativeWindow::~OffscreenNativeWindow() {
    printf("%s\n",__PRETTY_FUNCTION__);
}

// overloads from BaseNativeWindow
int OffscreenNativeWindow::setSwapInterval(int interval) {
    printf("%s\n",__PRETTY_FUNCTION__);
    return 0;
}

int OffscreenNativeWindow::dequeueBuffer(BaseNativeWindowBuffer **buffer){
    printf("%s\n",__PRETTY_FUNCTION__);
    if(m_buffers[m_tailbuffer] == 0) {
        m_buffers[m_tailbuffer] = new OffscreenNativeWindowBuffer(width(), height(), m_format, m_usage);
        int err = m_alloc->alloc(m_alloc,
                        width(), height(), m_format,
                        m_buffers[m_tailbuffer]->usage,
                        &m_buffers[m_tailbuffer]->handle,
                        &m_buffers[m_tailbuffer]->stride);
        printf("buffer %i is at %p (native %p) err=%s handle=%i stride=%i\n", 
                m_tailbuffer, m_buffers[m_tailbuffer], (ANativeWindowBuffer*) m_buffers[m_tailbuffer],
                strerror(-err), m_buffers[m_tailbuffer]->handle, m_buffers[m_tailbuffer]->stride);
    }
    *buffer = m_buffers[m_tailbuffer];
    printf("dequeueing buffer %i %p\n",m_tailbuffer, m_buffers[m_tailbuffer]);
    m_tailbuffer++;
    if(m_tailbuffer == NUM_BUFFERS)
        m_tailbuffer = 0;
    return NO_ERROR;
}

int OffscreenNativeWindow::lockBuffer(BaseNativeWindowBuffer* buffer){
    OffscreenNativeWindowBuffer *buf = static_cast<OffscreenNativeWindowBuffer*>(buffer);
    printf("%s\n",__PRETTY_FUNCTION__);
    int err = m_gralloc->lock(m_gralloc, 
            buf->handle, buf->usage,
            0,0, m_width, m_height,
            &buf->vaddr
            );
    printf("lock %s vaddr %p\n", strerror(-err), buf->vaddr);
    return NO_ERROR;
}

int OffscreenNativeWindow::queueBuffer(BaseNativeWindowBuffer* buffer){
    OffscreenNativeWindowBuffer* buf = static_cast<OffscreenNativeWindowBuffer*>(buffer);
    m_frontbuffer++;
    if(m_frontbuffer == NUM_BUFFERS)
        m_frontbuffer = 0;
    int res = 0;
    //fixme
    printf("%s %s\n",__PRETTY_FUNCTION__,strerror(-res));
    return NO_ERROR;
}

int OffscreenNativeWindow::cancelBuffer(BaseNativeWindowBuffer* buffer){
    printf("%s\n",__PRETTY_FUNCTION__);
    return 0;
}

unsigned int OffscreenNativeWindow::width() const {
    printf("%s value: %i\n",__PRETTY_FUNCTION__, m_width);
    return m_width;
}

unsigned int OffscreenNativeWindow::height() const {
    printf("%s value: %i\n",__PRETTY_FUNCTION__, m_height);
    return m_height;
}

unsigned int OffscreenNativeWindow::format() const {
    printf("%s value: %i\n",__PRETTY_FUNCTION__, m_format);
    return m_format;
}

unsigned int OffscreenNativeWindow::defaultWidth() const {
    printf("%s value: %i\n",__PRETTY_FUNCTION__, m_defaultWidth);
    return m_defaultWidth;
}

unsigned int OffscreenNativeWindow::defaultHeight() const {
    printf("%s value: %i\n",__PRETTY_FUNCTION__, m_defaultHeight);
    return m_defaultHeight;
}

unsigned int OffscreenNativeWindow::queueLength() const {
    printf("%s\n",__PRETTY_FUNCTION__);
    return 1;
}

unsigned int OffscreenNativeWindow::type() const {
    printf("%s\n",__PRETTY_FUNCTION__);
    return NATIVE_WINDOW_SURFACE_TEXTURE_CLIENT;
}

unsigned int OffscreenNativeWindow::transformHint() const {
    printf("%s\n",__PRETTY_FUNCTION__);
    return 0;
}

int OffscreenNativeWindow::setBuffersFormat(int format) {
    printf("%s format %i\n",__PRETTY_FUNCTION__, format);
    return NO_ERROR;
}

int OffscreenNativeWindow::setBuffersDimensions(int width, int height) {
    printf("%s size %ix%i\n",__PRETTY_FUNCTION__, width, height);
    return NO_ERROR;
}

int OffscreenNativeWindow::setUsage(int usage) {
    printf("%s usage %i\n",__PRETTY_FUNCTION__, usage);
    m_usage = usage;
    return NO_ERROR;
}
