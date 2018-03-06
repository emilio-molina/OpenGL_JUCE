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

struct Vertex
{
    float position[3];  // position of the vertex
    float normal[3];    // normal vector of the vertex
    float colour[4];    // color (rgb + alpha)
    float texCoord[2];  // coordinate of texture
};

class SolidSphereGenerator
{
public:
    std::vector<Vertex> templateVertices;
    std::vector<int> templateIndices;
    
    SolidSphereGenerator(unsigned int rings,
                         unsigned int sectors)
    {
        float const R = 1./(float)(rings-1);
        float const S = 1./(float)(sectors-1);
        for(int r = 0; r < rings; r++) {
            for(int s = 0; s < sectors; s++) {
                float y = sin( -M_PI_2 + M_PI * r * R );
                float x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
                float z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
                templateVertices.push_back(
                                           {
                                               { x, y, z},
                                               { x, y, z},
                                               { 1.0f, 1.0f, 1.0f, 1.0f},
                                               { s * S, r * R}
                                           }
                                           );
            }
        }
        for(int r = 0; r < rings; r++) {
            for(int s = 0; s < sectors; s++) {
                templateIndices.push_back(r * sectors + s);
                templateIndices.push_back(r * sectors + (s+1));
                templateIndices.push_back((r+1) * sectors + (s+1));
                templateIndices.push_back((r+1) * sectors + s);
            }
        }
    }
    
    void generateSphere(int sphereId, float offsetX, float offsetY, float offsetZ,
                        float colourR, float colourG, float colourB, float alpha,
                        float radius, Array<Vertex> &vertices, Array<int> &sphereIds,
                        Array<int> &indices) {
        int startIndex = (int)vertices.size();
        for (auto v: templateVertices) {
            vertices.add(
                         {
                             { v.position[0] * radius + offsetX,
                                 v.position[1] * radius + offsetY,
                                 v.position[2] * radius + offsetZ},
                             { v.normal[0], v.normal[1], v.normal[2]},
                             {colourR, colourG, colourB, alpha},
                             {v.texCoord[0], v.texCoord[1]}
                         }
                         );
            sphereIds.add(sphereId);
        }
        for (auto i: templateIndices) {
            indices.add(i + startIndex);
        }
    }
};

struct Uniforms
{
    Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
    {
        projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix");
        viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix");
        lightPosition    = createUniform (openGLContext, shaderProgram, "lightPosition");
    }
    
    ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, lightPosition;
    
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



class ScatterPlot    : public OpenGLAppComponent
{
public:
    Camera* _camera;
    std::vector<glm::vec3> spherePositions;
    ScatterPlot();
    ~ScatterPlot();
    void initialise() override;
    void shutdown() override;
    void render() override;
    void paint (Graphics&) override;
    void resized() override;
    Matrix3D<float> getProjectionMatrix() const;
    Matrix3D<float> getViewMatrix() const;
    void mouseDown (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& d) override;
    void createShaders();
    void auxRender1();
    void auxRender2();
    void auxRender3();
    
private:
    Random r;
    Draggable3DOrientation draggableOrientation;
    float draggingX;
    float draggingY;
    bool initialized;
    // OpenGL variables:
    Array<Vertex> vertices;
    Array<int> indices;
    Array<int> sphereId;
    GLuint vertexBuffer, indexBuffer;
    const char* vertexShader;
    const char* fragmentShader;
    float zoomValue;
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;
    ScopedPointer<Uniforms> uniforms;
    String newVertexShader, newFragmentShader;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScatterPlot)
};
