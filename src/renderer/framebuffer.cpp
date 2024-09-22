#include "framebuffer.h"
#include <iostream>

Framebuffer::Framebuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer)
    : m_width(width), m_height(height), m_useDepthBuffer(useDepthBuffer)
{
    initFrameBuffer(width, height, numColorAttachments);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
    if (m_useDepthBuffer) {
        glDeleteRenderbuffers(1, &m_depthBuffer);
    }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::getColorAttachment(unsigned int index) const {
    if (index >= m_colorAttachments.size()) {
        std::cerr << "[Error] Framebuffer::getColorAttachment: Invalid color attachment index!" << std::endl;
        return 0;
    }
    return m_colorAttachments[index];
}

unsigned int Framebuffer::getDepthAttachment() const {
    if (!m_useDepthBuffer) {
        std::cerr << "[Error] Framebuffer::getDepthAttachment: No depth buffer available!" << std::endl;
        return 0;
    }
    return m_depthBuffer;
}

void Framebuffer::resize(unsigned int width, unsigned int height) {
    m_width = width;
    m_height = height;
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
    if (m_useDepthBuffer) {
        glDeleteRenderbuffers(1, &m_depthBuffer);
    }
    initFrameBuffer(width, height, m_colorAttachments.size());
}

void Framebuffer::initFrameBuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments) {
    // Create framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create color attachments
    m_colorAttachments.resize(numColorAttachments);
    glGenTextures(numColorAttachments, m_colorAttachments.data());

    for (unsigned int i = 0; i < numColorAttachments; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_colorAttachments[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr); // Color texture format
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachments[i], 0);
    }

    // Optionally create a depth buffer
    if (m_useDepthBuffer) {
        glGenTextures(1, &m_depthBuffer);
        glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    }

    // Specify the list of draw buffers
    std::vector<GLenum> drawBuffers(numColorAttachments);
    for (unsigned int i = 0; i < numColorAttachments; ++i) {
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(numColorAttachments, drawBuffers.data());

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Error] Framebuffer::init: Framebuffer is not complete!\n";
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
