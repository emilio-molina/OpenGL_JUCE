//
//  Bloom.hpp
//  OpenGL_JUCE - App
//
//  Created by Chebonenko Vitali on 3/8/18.
//

#ifndef Bloom_hpp
#define Bloom_hpp

#include <stdio.h>
#include "../JuceLibraryCode/JuceHeader.h"

class Bloom
{
private:
    OpenGLFrameBuffer mainBuffer;
    OpenGLFrameBuffer glowBuffer;
    
public:
    Bloom();
    void On(OpenGLFrameBuffer &fb);
};


#endif /* Bloom_hpp */
