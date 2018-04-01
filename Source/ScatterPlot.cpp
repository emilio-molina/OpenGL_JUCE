/*
 ==============================================================================
 
 ScatterPlot.cpp
 Created: 27 Jan 2018 4:34:23pm
 Author:  Emilio Molina
 
 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "ScatterPlot.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace glm;

/* Generate a template for one sphere
 *
 * @param rings Number of rings in mesh
 * @param sectors Number of sectors in mesh
 */
SolidSphereGenerator::SolidSphereGenerator(unsigned int rings,
                                           unsigned int sectors)
{
    printf("%f %f/n", M_PI, M_PI_2);
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

/* Generate a new sphere with specific colour and size
 *
 * @param sphereId Id given to this new sphere
 * @param offsetX X position of center
 * @param offsetY Y position of center
 * @param offsetZ Z position of center
 * @param alpha Transparency
 * @param radius Radius of sphere
 * @param vertices Output array where sphere vertices will be appended
 * @param sphereIds Output array where each vertex index is assigned an sphereId
 * @param indices Output array with indices
 */
void SolidSphereGenerator::generateSphere(int sphereId, float offsetX,
                                          float offsetY, float offsetZ,
                                          float colourR, float colourG,
                                          float colourB, float alpha,
                                          float radius, Array<Vertex> &vertices,
                                          Array<int> &sphereIds, Array<int> &indices) {
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


/* ScatterPlot component initializer
 *
 */
ScatterPlot::ScatterPlot()
{
    // apparently openGL 3.2 is the highest supported by JUCE
    //openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
    
    
    _camera = new Camera();
    setSize (800, 600);
    zoomValue = 5.0f;
    draggingX = 0.0f;
    draggingY = 0.0f;
    position = nullptr;
    normal = nullptr;
    vertex = nullptr;
    textureCoordIn = nullptr;
    sourceColour = nullptr;
    SolidSphereGenerator sphereGenerator(10, 10);
    
    int sphere_id = 0;

    Random r;
    
    // Generate a template sphere in position 0.0f and size 1.0f
    sphereGenerator.generateSphere(sphere_id,
                                   0.f,
                                   0.f,
                                   0.f,
                                   1.0f,
                                   1.0f,
                                   1.0f,
                                   1.0f,
                                   0.01f,
                                   vertices,
                                   sphereId,
                                   indices);
    
    for (int j=0; j<20000; j++) {
                float ix = (r.nextFloat() - 0.5f) * 20;
                float iy = (r.nextFloat() - 0.5f) * 20;
                float iz = (r.nextFloat() - 0.5f) * 20;
                float x = ix * 0.1f;
                float y = iy * 0.1f;
                float z = iz * 0.1f;
                glm::vec3 v(x, y, z);
                spherePositions.push_back(v);
    }
    


    
    
}

Matrix3D<float> ScatterPlot::getProjectionMatrix() const
{
    float w = 1.0f / (zoomValue + 0.1f);
    //float w = 1.0f / (5.0f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio (false);
    return Matrix3D<float>::fromFrustum (-w, w, -h, h, 1.0f, 30.0f);
}

Matrix3D<float> ScatterPlot::getViewMatrix() const
{
    auto viewMatrix = //Matrix3D<float>(Vector3D<float> (0.0f, 0.0f, zoomValue)) *
    draggableOrientation.getRotationMatrix()
    * Vector3D<float> (0.0f, 0.0f, -10.0f);
    return viewMatrix;
}




void ScatterPlot::render()
{
    auxRender1();
    auxRender2();
    auxRender3();
}

ScatterPlot::~ScatterPlot()
{
    shutdownOpenGL();
}

void ScatterPlot::paint (Graphics& g)
{
}

void ScatterPlot::resized()
{
    draggableOrientation.setViewport (getLocalBounds());
    
}


//=== OpenGL auxiliar functions

/** @brief Create OpenGL shaders
 *
 * Vertex shader: Graphic-card routine determining how vertices modify their
 *                coordinates, as a function of camera position, etc.
 *
 * Fragment shader: Graphic-card routine determining how pixels are colored
 *                  depending on their position, colors, texture, light, etc.
 */


std::string readShaderCode(const char* fileName)
{
    std::ifstream meInput(fileName);
    if (!meInput.good())
    {
        std::cout << "File failed to read " << fileName << std::endl;
        exit(1);
    }
    return std::string(
                       std::istreambuf_iterator<char>(meInput),
                       std::istreambuf_iterator<char>());
}



void ScatterPlot::createLambertShader()
{
    
    //TODO: Why JUCE dosn't have separate shader creation. Vertex shader could be reused whe don't need it to create for each shader 
    lambertShader =  new OpenGLShaderProgram (openGLContext);
    
    String statusText;
    // Note that BinaryData.h is automatically generated by Projucer using resource files
    // In case shaders want to be modified, I suggest to re-generate xcode project with Projucer
    lambertShader->addShader( BinaryData::fragmentShader_glsl , GL_FRAGMENT_SHADER);
    lambertShader->addShader( BinaryData::vertexShader_glsl , GL_VERTEX_SHADER);
    lambertShader->link();
    
    if (lambertShader->link())
    {
        lambertUniforms   = new Uniforms (openGLContext, *lambertShader);
        
        if (openGLContext.extensions.glGetAttribLocation (lambertShader->getProgramID(), "position") < 0)
            position      = nullptr;
        else
            position      = new OpenGLShaderProgram::Attribute (*lambertShader,    "position");
        
        if (openGLContext.extensions.glGetAttribLocation (lambertShader->getProgramID(), "vertex") < 0)
            vertex      = nullptr;
        else
            vertex  = new OpenGLShaderProgram::Attribute (*lambertShader,    "vertex");
        
        if (openGLContext.extensions.glGetAttribLocation (lambertShader->getProgramID(), "sourceColour") < 0)
            sourceColour      = nullptr;
        else
            sourceColour  = new OpenGLShaderProgram::Attribute (*lambertShader,    "sourceColour");
        
        if (openGLContext.extensions.glGetAttribLocation (lambertShader->getProgramID(), "normal") < 0)
            normal      = nullptr;
        else
            normal      = new OpenGLShaderProgram::Attribute (*lambertShader,    "normal");
        if (openGLContext.extensions.glGetAttribLocation (lambertShader->getProgramID(), "textureCoordIn") < 0)
            textureCoordIn      = nullptr;
        else
            textureCoordIn      = new OpenGLShaderProgram::Attribute (*lambertShader,    "textureCoordIn");
        
        statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText = lambertShader->getLastError();
    }
}

void ScatterPlot::createHoverShader()
{
    
    //TODO: Why JUCE dosn't have separate shader creation. Vertex shader could be reused whe don't need it to create for each shader
    hoverShader =  new OpenGLShaderProgram (openGLContext);
    
    String statusText;
    // Note that BinaryData.h is automatically generated by Projucer using resource files
    // In case shaders want to be modified, I suggest to re-generate xcode project with Projucer
    hoverShader->addShader( BinaryData::hoverShader_glsl , GL_FRAGMENT_SHADER);
    hoverShader->addShader( BinaryData::vertexShader_glsl , GL_VERTEX_SHADER);
    hoverShader->link();
    
    if (hoverShader->link())
    {
        hoverUniforms   = new Uniforms (openGLContext, *hoverShader);
        
        if (openGLContext.extensions.glGetAttribLocation (hoverShader->getProgramID(), "position") < 0)
            position      = nullptr;
        else
            position      = new OpenGLShaderProgram::Attribute (*hoverShader,    "position");
        
        if (openGLContext.extensions.glGetAttribLocation (hoverShader->getProgramID(), "vertex") < 0)
            vertex      = nullptr;
        else
            vertex  = new OpenGLShaderProgram::Attribute (*hoverShader,    "vertex");
        
        if (openGLContext.extensions.glGetAttribLocation (hoverShader->getProgramID(), "sourceColour") < 0)
            sourceColour      = nullptr;
        else
            sourceColour  = new OpenGLShaderProgram::Attribute (*hoverShader,    "sourceColour");
        
        if (openGLContext.extensions.glGetAttribLocation (hoverShader->getProgramID(), "normal") < 0)
            normal      = nullptr;
        else
            normal      = new OpenGLShaderProgram::Attribute (*hoverShader,    "normal");
        if (openGLContext.extensions.glGetAttribLocation (hoverShader->getProgramID(), "textureCoordIn") < 0)
            textureCoordIn      = nullptr;
        else
            textureCoordIn      = new OpenGLShaderProgram::Attribute (*hoverShader,    "textureCoordIn");
        
        statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText = hoverShader->getLastError();
    }
}

/** @brief OpenGL initialization function called only once
 */
void ScatterPlot::initialise()
{
    
    createLambertShader();
    createHoverShader();
    
    // We only need to define this data once ----
    // TODO: Explain
    
    openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                           vertices.size() * sizeof (Vertex),
                                           vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);
    
    
    openGLContext.extensions.glGenBuffers (1, &indexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                           indices.size() * sizeof (juce::uint32),
                                           indices.getRawDataPointer(), GL_DYNAMIC_DRAW);
}

/** @brief Shutdown OpenGL
 */
void ScatterPlot::shutdown()
{
    lambertShader = nullptr;
}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender1() {
    // Stuff to be done before defining your triangles
    jassert (OpenGLHelpers::isContextActive());
    const float desktopScale = (float) openGLContext.getRenderingScale();
    OpenGLHelpers::clear (Colour::greyLevel (0.05f));
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));

}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender2() {
    // ************************************************

    // TODO: Explain
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    openGLContext.extensions.glBindBuffer (GL_UNIFORM_BUFFER, positionBuffer);
    
    lambertShader->use();
    
    if (lambertUniforms->projectionMatrix != nullptr)
        lambertUniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
    
    if (lambertUniforms->viewMatrix != nullptr)
        lambertUniforms->viewMatrix->setMatrix4(glm::value_ptr(_camera->getWorldToViewMatrix()), 1, false);
    
    if (lambertUniforms->lightPosition != nullptr)
        lambertUniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
    
    if (lambertUniforms->scale != nullptr)
        lambertUniforms->scale->set(2.f);
    
    // Now prepare this information to be drawn
    if (vertex != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (vertex->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
        openGLContext.extensions.glEnableVertexAttribArray (vertex->attributeID);
    }
    
    if (normal != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
        openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
    }
    
    if (sourceColour != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
        openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
    }
    
    if (textureCoordIn != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
        openGLContext.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
    }
    
    
    for (int n = 0; n < spherePositions.size(); n++)
    {
        if (lambertUniforms->position != nullptr)
            lambertUniforms->position->set(spherePositions[n].x, spherePositions[n].y, spherePositions[n].z, 1.0);
            glDrawElements (GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    hoverShader->use();
    
    if (hoverUniforms->projectionMatrix != nullptr)
        hoverUniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
    
    if (hoverUniforms->viewMatrix != nullptr)
        hoverUniforms->viewMatrix->setMatrix4(glm::value_ptr(_camera->getWorldToViewMatrix()), 1, false);
    
    if (hoverUniforms->lightPosition != nullptr)
        hoverUniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
    
    for (int n = 0; n < hoveredSpherePositions.size(); n++)
    {
        if (hoverUniforms->position != nullptr)
            hoverUniforms->position->set(hoveredSpherePositions[n].x, hoveredSpherePositions[n].y, hoveredSpherePositions[n].z, 1.0);
        
        if (hoverUniforms->scale != nullptr)
            hoverUniforms->scale->set(1.f);
        
        glDrawElements (GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    
}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender3() {
    if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
    if (vertex != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (vertex->attributeID);
    if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
    if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
    if (textureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
    
    // Reset the element buffers so child Components draw correctly
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
    //openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
    repaint();
}


void ScatterPlot::mouseDown (const MouseEvent& e)
{
    _camera->setMousePressPos(glm::vec2(e.getPosition().x, e.getPosition().y));
    
    //draggableOrientation.mouseDown (e.getPosition());
    
}

void ScatterPlot::mouseDrag (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown()){
        _camera->rotate(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
    }
    if (e.mods.isMiddleButtonDown()){
        _camera->pan(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
    }
    if (e.mods.isRightButtonDown()){
        _camera->zoom(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
    }
    //draggableOrientation.mouseDrag (e.getPosition());
}

void ScatterPlot::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& d)
{
    // _camera->zoom(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
    //zoomValue += d.deltaY;
    //zoomValue = jmin(jmax(zoomValue, 0.1f), 30.0f);
}



/* Code taken from:
 * https://github.com/opengl-tutorials/ogl/blob/master/misc05_picking/misc05_picking_custom.cpp#L30
 * Convert screen positions to a world space ray
 * 
 * @param x Camera x position
 * @param y Camera y position
 * @param ViewMatrix Camera orientation
 * @param ProjectionMatrix Camera parameters (ratio, field of view, near and far planes)
 * @param out_origin Ouput origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
 * @param out_direction Output direction, in world space, of the ray that goes "through" the mouse
 *
 */
void ScreenPosToWorldRay(
                         float x, float y,
                         glm::mat4 ViewMatrix,
                         glm::mat4 ProjectionMatrix,
                         glm::vec3& out_origin,
                         glm::vec3& out_direction){
    
    // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
    glm::vec4 lRayStart_NDC(
                            x,
                            y,
                            -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
                            1.0f
                            );
    glm::vec4 lRayEnd_NDC(
                          x,
                          y,
                          0.0,
                          1.0f
                          );
    
    
    // The Projection matrix goes from Camera Space to NDC.
    // So inverse(ProjectionMatrix) goes from NDC to Camera Space.
    glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);
    
    // The View Matrix goes from World Space to Camera Space.
    // So inverse(ViewMatrix) goes from Camera Space to World Space.
    glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
    
    glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera/=lRayStart_camera.w;
    glm::vec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world /=lRayStart_world .w;
    glm::vec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera  /=lRayEnd_camera  .w;
    glm::vec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world   /=lRayEnd_world   .w;
    
    
    // Faster way (just one inverse)
    //glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
    //glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
    //glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;
    
    
    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    lRayDir_world = glm::normalize(lRayDir_world);
    
    
    out_origin = glm::vec3(lRayStart_world);
    out_direction = glm::normalize(lRayDir_world);
}


/* Test if a specific ray intersects a specific 3d box
 *
 * @param ray_origin Ray origin, in world space
 * @param ray_direction Ray direction (NOT target position!), in world space. Must be normalize()'d.
 * @param aabb_min Minimum X,Y,Z coords of the mesth when not transformed at all
 * @param aabb_max Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case
 * @param ModelMatrix Transformation applied to the mesh (which will thus be also applied to its bounding box)
 * @param intersection_distance Output distance between ray_origin and the intersection with the OBB
 */
bool TestRayOBBIntersection(
                            glm::vec3 ray_origin,
                            glm::vec3 ray_direction,
                            glm::vec3 aabb_min,
                            glm::vec3 aabb_max,
                            glm::mat4 ModelMatrix,
                            float& intersection_distance){
    
    // Intersection method from Real-Time Rendering and Essential Mathematics for Games
    
    float tMin = 0.0f;
    float tMax = 100000.0f;
    
    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
    
    glm::vec3 delta = OBBposition_worldspace - ray_origin;
    
    // Test intersection with the 2 planes perpendicular to the OBB's X axis
    {
        glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
        float e = glm::dot(xaxis, delta);
        float f = glm::dot(ray_direction, xaxis);
        
        if ( fabs(f) > 0.001f ){ // Standard case
            
            float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
            float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
            // t1 and t2 now contain distances betwen ray origin and ray-plane intersections
            
            // We want t1 to represent the nearest intersection,
            // so if it's not the case, invert t1 and t2
            if (t1>t2){
                float w=t1;t1=t2;t2=w; // swap t1 and t2
            }
            
            // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
            if ( t2 < tMax )
                tMax = t2;
            // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
            if ( t1 > tMin )
                tMin = t1;
            
            // And here's the trick :
            // If "far" is closer than "near", then there is NO intersection.
            // See the images in the tutorials for the visual explanation.
            if (tMax < tMin )
                return false;
            
        }else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
            if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
                return false;
        }
    }
    
    
    // Test intersection with the 2 planes perpendicular to the OBB's Y axis
    // Exactly the same thing than above.
    {
        glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
        float e = glm::dot(yaxis, delta);
        float f = glm::dot(ray_direction, yaxis);
        
        if ( fabs(f) > 0.001f ){
            
            float t1 = (e+aabb_min.y)/f;
            float t2 = (e+aabb_max.y)/f;
            
            if (t1>t2){float w=t1;t1=t2;t2=w;}
            
            if ( t2 < tMax )
                tMax = t2;
            if ( t1 > tMin )
                tMin = t1;
            if (tMin > tMax)
                return false;
        }else{
            if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
                return false;
        }
    }
    
    
    // Test intersection with the 2 planes perpendicular to the OBB's Z axis
    // Exactly the same thing than above.
    {
        glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
        float e = glm::dot(zaxis, delta);
        float f = glm::dot(ray_direction, zaxis);
        
        if ( fabs(f) > 0.001f ){
            
            float t1 = (e+aabb_min.z)/f;
            float t2 = (e+aabb_max.z)/f;
            
            if (t1>t2){float w=t1;t1=t2;t2=w;}
            
            if ( t2 < tMax )
                tMax = t2;
            if ( t1 > tMin )
                tMin = t1;
            if (tMin > tMax)
                return false;
            
        }else{
            if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
                return false;
        }
    }
    
    intersection_distance = tMin;
    return true;
}



void ScatterPlot::mouseMove (const MouseEvent& e)
{
    float x = ((float)e.getPosition().getX() / getWidth() - 0.5f) * 2;
    float y = ((float)e.getPosition().getY() / getHeight() - 0.5f) * -2;
    Logger::writeToLog(String(x) + ", " + String(y));
    vec4 screenCoords(x, y, 1.0f, 1.0f);
    Logger::writeToLog(String(screenCoords.x) + ", " + String(screenCoords.y) + ", " + String(screenCoords.z));
    glm::vec3 out_origin, out_direction;
    ScreenPosToWorldRay(x, y,
                        make_mat4(getViewMatrix().mat),
                        make_mat4(getProjectionMatrix().mat),
                        out_origin,
                        out_direction);
    
    for(int i=0; i<spherePositions.size(); i++){
        
        float intersection_distance; // Output of TestRayOBBIntersection()
        glm::vec3 aabb_min(-0.01f, -0.01f, -0.01f);
        glm::vec3 aabb_max( 0.01f,  0.01f,  0.01f);
        
        // The ModelMatrix transforms :
        // - the mesh to its desired position and orientation
        // - but also the AABB (defined with aabb_min and aabb_max) into an OBB
        glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), spherePositions[i]);
        glm::mat4 ModelMatrix = TranslationMatrix;
        
        
        if (TestRayOBBIntersection(out_origin,
                                    out_direction,
                                    aabb_min,
                                    aabb_max,
                                    ModelMatrix,
                                    intersection_distance))
        {
            if( hoveredSpherePositions.size() > 0)
            {
                spherePositions.push_back(hoveredSpherePositions.front());
                hoveredSpherePositions.erase(hoveredSpherePositions.begin());
            }
            hoveredSpherePositions.push_back(spherePositions[i]);
            spherePositions.erase(spherePositions.begin() + i);
            //Logger::writeToLog("intersect: " + String(i));
            break;
        }
    }
}


