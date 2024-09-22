#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

class Framebuffer {
public:
    Framebuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer = true);
    ~Framebuffer();

    void bind() const;
    void unbind() const;

    unsigned int getColorAttachment(unsigned int index) const;
    unsigned int getDepthAttachment() const;

    void resize(unsigned int width, unsigned int height);

private:
    unsigned int m_fbo;
    std::vector<unsigned int> m_colorAttachments;
    unsigned int m_depthBuffer = 0;
    unsigned int m_width, m_height;
    bool m_useDepthBuffer;

    void initFrameBuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments);
};

#endif // FRAMEBUFFER_H
