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
    GLuint texColorBuffer;
    glGenFramebuffers(1, &texColorBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, texColorBuffer);
    
    glTexImage2D
    (
                 GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(
                           GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0
    );
    
    
    
}
