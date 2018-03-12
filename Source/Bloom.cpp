//
//  Bloom.cpp
//  OpenGL_JUCE - App
//
//  Created by Chebonenko Vitali on 3/8/18.
//


#include "Bloom.hpp"

#define cpGenFramebuffers(a, b) \
((GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object) ? \
glGenFramebuffers((a), (b)) : \
glGenFramebuffersEXT((a), (b)))

Bloom::Bloom()
{}

void Bloom::On(OpenGLFrameBuffer &fb)
{
    GLuint frameBuffer;
    glGenFramebuffers(0, &frameBuffer);
}
