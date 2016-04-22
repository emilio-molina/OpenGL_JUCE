/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public OpenGLAppComponent
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (800, 600);
    }

    ~MainContentComponent()
    {
        shutdownOpenGL();
    }

    void initialise() override
    {
        GLfloat verts[]
        {
            +0.0f, +1.0f,
            -1.0f, -1.0f,
            +1.0f, -1.0f,
        };

        // Now we have to copy these vertices to my graphic card
        GLuint myBufferID;
        openGLContext.extensions.glGenBuffers(1, &myBufferID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, myBufferID);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Vertex attributes (needed to draw the triangle)
        openGLContext.extensions.glEnableVertexAttribArray(0);
        openGLContext.extensions.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    void shutdown() override
    {
    }

    void render() override
    {
        OpenGLHelpers::clear (Colours::black);
        glViewport(0, 0, getWidth(), getHeight());
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================

    // private member variables



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
