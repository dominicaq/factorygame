#include "framebuffer.h"

Framebuffer::Framebuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer) {
    m_data.pack(width, height, numColorAttachments, useDepthBuffer);
    initFrameBuffer(width, height, numColorAttachments, useDepthBuffer);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(MAX_ATTACHMENTS, m_colorAttachments);
    if (m_depthBuffer > 0) {
        glDeleteTextures(1, &m_depthBuffer);
    }
}

void Framebuffer::bind() const {
    unsigned int width, height;
    m_data.getDimensions(width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::getColorAttachment(unsigned int index) const {
    unsigned int numColorAttachments = m_data.getNumColorAttachments();

    if (index >= numColorAttachments) {
        std::cerr << "[Error] Framebuffer::getColorAttachment: Invalid color attachment index!" << "\n";
        return 0;
    }
    return m_colorAttachments[index];
}

unsigned int Framebuffer::getDepthAttachment() const {
    if (m_depthBuffer == 0) {
        std::cerr << "[Error] Framebuffer::getDepthAttachment: No depth buffer available!" << "\n";
        return 0;
    }
    return m_depthBuffer;
}

void Framebuffer::resetDepthAttachment() const {
    if (m_depthBuffer == 0) {
        std::cerr << "[Error] Framebuffer::resetDepthAttachment: No depth buffer available!" << "\n";
        return;
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void Framebuffer::resize(unsigned int width, unsigned int height) {
    unsigned int numColorAttachments = m_data.getNumColorAttachments();
    bool depthFlag = m_data.getDepthFlag();

    // Update the packed data
    m_data.pack(width, height, numColorAttachments, depthFlag);

    // Reinitialize the framebuffer
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(numColorAttachments, m_colorAttachments);
    if (m_depthBuffer > 0) {
        glDeleteTextures(1, &m_depthBuffer);
    }

    initFrameBuffer(width, height, numColorAttachments, depthFlag);
}

void Framebuffer::initFrameBuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer) {
    // Create framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (numColorAttachments > 0) {
        // Create color attachments
        glGenTextures(numColorAttachments, m_colorAttachments);

        for (unsigned int i = 0; i < numColorAttachments; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_colorAttachments[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachments[i], 0);
        }

        // Specify the list of draw buffers
        GLenum drawBuffers[MAX_ATTACHMENTS];
        for (unsigned int i = 0; i < numColorAttachments; ++i) {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(numColorAttachments, drawBuffers);
    } else {
        // No color attachments, disable drawing to color buffers
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // Create depth buffer if needed
    if (useDepthBuffer) {
        glGenTextures(1, &m_depthBuffer);
        glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    }

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Error] Framebuffer::initFrameBuffer: Framebuffer is not complete!\n";
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
