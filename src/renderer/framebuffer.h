#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <iostream>
#include <cstdint>

// TODO: Subject to change
#define MAX_ATTACHMENTS 5

class Framebuffer {
public:
    Framebuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer);
    ~Framebuffer();

    void initFrameBuffer(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool useDepthBuffer);

    void bind() const;
    void unbind() const;

    unsigned int getColorAttachment(unsigned int index) const;
    unsigned int getDepthAttachment() const;
    void resetDepthAttachment() const;

    void resize(unsigned int width, unsigned int height);

private:
    unsigned int m_fbo;
    unsigned int m_colorAttachments[MAX_ATTACHMENTS];
    unsigned int m_depthBuffer = 0;

    // Manage packed framebuffer data
    struct FramebufferData {
        uint64_t packedData;

        // Pack data: [width | height | numAttachments | depthFlag]
        void pack(unsigned int width, unsigned int height, unsigned int numColorAttachments, bool depthFlag) {
            packedData = (uint64_t(width) & 0xFFFF) |
                         ((uint64_t(height) & 0xFFFF) << 16) |
                         ((uint64_t(numColorAttachments) & 0xF) << 32) |
                         (uint64_t(depthFlag) << 36);
        }

        // Unpack getter functions
        void getDimensions(unsigned int& width, unsigned int& height) const {
            width = packedData & 0xFFFF;
            height = (packedData >> 16) & 0xFFFF;
        }

        unsigned int getNumColorAttachments() const {
            return (packedData >> 32) & 0xF;
        }

        bool getDepthFlag() const {
            return (packedData >> 36) & 0x1;
        }
    };

    FramebufferData m_data;  // Holds packed data
};

#endif // FRAMEBUFFER_H
