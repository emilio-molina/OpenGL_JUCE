/*
 ==============================================================================
 
 ScatterPlot.h
 Created: 27 Jan 2018 4:34:23pm
 Author:  Emilio Molina
 
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.hpp"
#include "Bloom.hpp"


#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif


#ifndef M_PI_2
    #define M_PI_2 1.57079632679489661923
#endif


/* Vertex struct for OpenGL buffers
 *
 */
struct Vertex
{
    float position[3];  // position of the vertex
    float normal[3];    // normal vector of the vertex
    float colour[4];    // color (rgb + alpha)
    float texCoord[2];  // coordinate of texture
};


/* Class to generate vertices of a sphere.
 * taken from:
 * https://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c/5989676#5989676
 */
class SolidSphereGenerator
{
public:
    std::vector<Vertex> templateVertices;
    std::vector<int> templateIndices;
    
    /* Create a single-sphere template with specific number of rings and sectors
     *
     */
    SolidSphereGenerator(unsigned int rings,
                         unsigned int sectors);
    
    /* Append to vertices, sphereIds, and indices all needed information to
     * create a new sphere.
     */
    void generateSphere(int sphereId, float offsetX, float offsetY, float offsetZ,
                    float colourR, float colourG, float colourB, float alpha,
                    float radius, Array<Vertex> &vertices, Array<int> &sphereIds,
                    Array<int> &indices);
};


/* Class taken from:
 * https://github.com/WeAreROLI/JUCE/blob/master/examples/GUI/OpenGLAppDemo.h#L283
 */
struct Uniforms
{
    Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
    {
        projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix");
        viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix");
        lightPosition    = createUniform (openGLContext, shaderProgram, "lightPosition");
        position         = createUniform (openGLContext, shaderProgram, "position");
        scale            = createUniform (openGLContext, shaderProgram, "scale");
    }
    
    ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, lightPosition, position, scale;
    
private:
    static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                        OpenGLShaderProgram& shaderProgram,
                                                        const char* uniformName)
    {
        if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
            return nullptr;
        
        return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
    }
};


/* Component for scatterplot of some thousands of spheres
 *
 */
class ScatterPlot    : public OpenGLAppComponent
{
public:
    
    Bloom* _bloomEffect;
    OpenGLFrameBuffer fb;
    Camera* _camera;
    std::vector<glm::vec3> spherePositions;
    std::vector<glm::vec3> hoveredSpherePositions;
    ScatterPlot();
    ~ScatterPlot();
    void initialise() override;
    void shutdown() override;
    void render() override;
    void auxRender1();
    void auxRender2();
    void auxRender3();
    void paint (Graphics&) override;
    void resized() override;
    void mouseDown (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& d) override;
    Matrix3D<float> getProjectionMatrix() const;
    Matrix3D<float> getViewMatrix() const;
    void createLambertShader();
    void createHoverShader();
    
private:

    ScopedPointer<OpenGLShaderProgram> lambertShader;
    ScopedPointer<OpenGLShaderProgram> hoverShader;
    
    Random r;
    Draggable3DOrientation draggableOrientation;
    float draggingX;
    float draggingY;
    bool initialized;
    // OpenGL variables:
    Array<Vertex> vertices;
    Array<int> indices;
    Array<int> sphereId;
    GLuint vertexBuffer, indexBuffer, positionBuffer;
    float zoomValue;
    ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn, vertex;
    ScopedPointer<Uniforms> lambertUniforms;
    ScopedPointer<Uniforms> hoverUniforms;
    String newVertexShader, newFragmentShader;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScatterPlot)
};
