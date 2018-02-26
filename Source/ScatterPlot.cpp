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
//==============================================================================
ScatterPlot::ScatterPlot()
{
    setSize (800, 600);
    zoomValue = 5.0f;
    draggingX = 0.0f;
    draggingY = 0.0f;
    initialized = false;
    position = nullptr;
    normal = nullptr;
    textureCoordIn = nullptr;
    sourceColour = nullptr;
    SolidSphereGenerator sphereGenerator(10, 10);
    /*for (int i=0; i<5000; i++) {
     sphereGenerator.generateSphere(
     i,
     (r.nextFloat() - 0.5f) * 10,
     (r.nextFloat() - 0.5f) * 10,
     (r.nextFloat() - 0.5f) * 10,
     r.nextFloat(),
     r.nextFloat(),
     r.nextFloat(),
     1.0f,
     0.075f,
     vertices,
     sphereId,
     indices);
     }*/
    int sphere_id = 0;
    //for (int ix=-10; ix<11; ix=ix+5) {
    //    for (int iy=-10; iy<11; iy=iy+5) {
    //        for (int iz=-10; iz<11; iz=iz+2) {
    Random r;
    for (int j=0; j<5000; j++) {
                float ix = (r.nextFloat() - 0.5f) * 20;
                float iy = (r.nextFloat() - 0.5f) * 20;
                float iz = (r.nextFloat() - 0.5f) * 20;
                float x = ix * 0.1f;
                float y = iy * 0.1f;
                float z = iz * 0.1f;
                glm::vec3 v(x, y, z);
                spherePositions.push_back(v);
                sphereGenerator.generateSphere(sphere_id,
                                               v.x,
                                               v.y,
                                               v.z,
                                               1.0f,
                                               1.0f,
                                               1.0f,
                                               1.0f,
                                               0.01f,
                                               vertices,
                                               sphereId,
                                               indices);
                sphere_id++;
    //        }
    //    }
    //}
    }
    initialized = true;
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



void ScatterPlot::mouseDown (const MouseEvent& e)
{
    draggableOrientation.mouseDown (e.getPosition());
}

void ScatterPlot::mouseDrag (const MouseEvent& e)
{
    draggableOrientation.mouseDrag (e.getPosition());
}


void ScreenPosToWorldRay(
                         float x, float y,
                         glm::mat4 ViewMatrix,               // Camera position and orientation
                         glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
                         glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
                         glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
){
    
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


bool TestRayOBBIntersection(
                            glm::vec3 ray_origin,        // Ray origin, in world space
                            glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                            glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
                            glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                            glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                            float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
){
    
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
    vec4 screenCoords(x, y, 0.0f, 1.0f);
    //Logger::writeToLog(String(worldCoords.x) + ", " + String(worldCoords.y) + ", " + String(worldCoords.z));
    glm::vec3 out_origin, out_direction;
    ScreenPosToWorldRay(x, y, make_mat4(getViewMatrix().mat), make_mat4(getProjectionMatrix().mat),
                        out_origin, out_direction);
    for(int i=0; i<spherePositions.size(); i++){
        
        float intersection_distance; // Output of TestRayOBBIntersection()
        glm::vec3 aabb_min(-0.01f, -0.01f, -0.01f);
        glm::vec3 aabb_max( 0.01f,  0.01f,  0.01f);
        
        // The ModelMatrix transforms :
        // - the mesh to its desired position and orientation
        // - but also the AABB (defined with aabb_min and aabb_max) into an OBB
        glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), spherePositions[i]);
        glm::mat4 ModelMatrix = TranslationMatrix;
        
        
        if ( TestRayOBBIntersection(
                                    out_origin,
                                    out_direction,
                                    aabb_min,
                                    aabb_max,
                                    ModelMatrix,
                                    intersection_distance)
            ){
            Logger::writeToLog("intersect: " + String(i));
            break;
        }
    }
}

void ScatterPlot::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& d)
{
    zoomValue += d.deltaY;
    zoomValue = jmin(jmax(zoomValue, 0.1f), 30.0f);
}

void ScatterPlot::render()
{
    auxRender1();
    auxRender2();
    glDrawElements (GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
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
void ScatterPlot::createShaders()
{
    // Here we define the shaders use to draw our triangle. They are very simple.
    
    vertexShader =
    "attribute vec4 position;\n"
    "attribute vec4 normal;\n"
    "attribute vec4 sourceColour;\n"
    "attribute vec2 texureCoordIn;\n"
    "\n"
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "uniform vec4 lightPosition;\n"
    "\n"
    "varying vec4 destinationColour;\n"
    "varying vec2 textureCoordOut;\n"
    "\n"
    "varying float lightIntensity;\n"
    "mat4 inverse(mat4 m) {\n"
    "   float\n"
    "   a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],\n"
    "   a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],\n"
    "   a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],\n"
    "   a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],\n"
    
    "   b00 = a00 * a11 - a01 * a10,\n"
    "   b01 = a00 * a12 - a02 * a10,\n"
    "   b02 = a00 * a13 - a03 * a10,\n"
    "   b03 = a01 * a12 - a02 * a11,\n"
    "   b04 = a01 * a13 - a03 * a11,\n"
    "   b05 = a02 * a13 - a03 * a12,\n"
    "   b06 = a20 * a31 - a21 * a30,\n"
    "   b07 = a20 * a32 - a22 * a30,\n"
    "   b08 = a20 * a33 - a23 * a30,\n"
    "   b09 = a21 * a32 - a22 * a31,\n"
    "   b10 = a21 * a33 - a23 * a31,\n"
    "   b11 = a22 * a33 - a23 * a32,\n"
    
    "   det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;\n"
    
    "   return mat4(\n"
    "               a11 * b11 - a12 * b10 + a13 * b09,\n"
    "               a02 * b10 - a01 * b11 - a03 * b09,\n"
    "               a31 * b05 - a32 * b04 + a33 * b03,\n"
    "               a22 * b04 - a21 * b05 - a23 * b03,\n"
    "               a12 * b08 - a10 * b11 - a13 * b07,\n"
    "               a00 * b11 - a02 * b08 + a03 * b07,\n"
    "               a32 * b02 - a30 * b05 - a33 * b01,\n"
    "               a20 * b05 - a22 * b02 + a23 * b01,\n"
    "               a10 * b10 - a11 * b08 + a13 * b06,\n"
    "               a01 * b08 - a00 * b10 - a03 * b06,\n"
    "               a30 * b04 - a31 * b02 + a33 * b00,\n"
    "               a21 * b02 - a20 * b04 - a23 * b00,\n"
    "               a11 * b07 - a10 * b09 - a12 * b06,\n"
    "               a00 * b09 - a01 * b07 + a02 * b06,\n"
    "               a31 * b01 - a30 * b03 - a32 * b00,\n"
    "               a20 * b03 - a21 * b01 + a22 * b00) / det;\n"
    "}\n"
    
    "void main()\n"
    "{\n"
    "    vec4 light = inverse(viewMatrix) * lightPosition;\n"
    "    lightIntensity = dot (light, normal);\n"
    "    destinationColour = sourceColour;\n"
    "    textureCoordOut = texureCoordIn;\n"
    "    gl_Position = projectionMatrix * viewMatrix * position;\n"
    //"    gl_Position = position;\n"
    "}\n";
    
    fragmentShader =
    "varying vec4 destinationColour;\n"
    "varying float lightIntensity;\n"
    "varying vec2 textureCoordOut;\n"
    "\n"
    "uniform sampler2D demoTexture;\n"
    "void main()\n"
    "{\n"
    "    float l = max (0.2, lightIntensity * 0.04); \n"
    "    vec4 c = destinationColour;\n"
    "    vec4 colour = vec4 (l * c[0], l * c[1], l * c[2], c[3]);\n"
    "    gl_FragColor = colour; \n"
    "}\n";
    
    ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
    String statusText;
    
    if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
        && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
        && newShader->link())
    {
        shader = newShader;
        shader->use();
        
        uniforms   = new Uniforms (openGLContext, *shader);
        
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "position") < 0)
            position      = nullptr;
        else
            position      = new OpenGLShaderProgram::Attribute (*shader,    "position");
        
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "sourceColour") < 0)
            sourceColour      = nullptr;
        else
            sourceColour  = new OpenGLShaderProgram::Attribute (*shader,    "sourceColour");
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "normal") < 0)
            normal      = nullptr;
        else
            normal      = new OpenGLShaderProgram::Attribute (*shader,    "normal");
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "textureCoordIn") < 0)
            textureCoordIn      = nullptr;
        else
            textureCoordIn      = new OpenGLShaderProgram::Attribute (*shader,    "textureCoordIn");
        
        statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText = newShader->getLastError();
    }
}

/** @brief OpenGL initialization function called only once
 */
void ScatterPlot::initialise()
{
    createShaders();
}

/** @brief Shutdown OpenGL
 */
void ScatterPlot::shutdown()
{
    shader = nullptr;
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
    shader->use();
    if (uniforms->projectionMatrix != nullptr)
        uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
    
    if (uniforms->viewMatrix != nullptr)
        uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);
    
    if (uniforms->lightPosition != nullptr)
        uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
    openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender2() {
    // ************************************************
    
    // Now prepare this information to be drawn
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                           static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                                           vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);
    
    openGLContext.extensions.glGenBuffers (1, &indexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                           static_cast<GLsizeiptr> (static_cast<size_t> (indices.size()) * sizeof (juce::uint32)),
                                           indices.getRawDataPointer(), GL_DYNAMIC_DRAW);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    
    if (position != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
        openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
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
}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender3() {
    if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
    if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
    if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
    if (textureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
    
    // Reset the element buffers so child Components draw correctly
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    
    openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
    openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
    repaint();
}




/*
 
 
 int main( void )
 {
 
 
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
 
 
 
	// Generate positions & rotations for 100 monkeys
	std::vector<glm::vec3> positions(100);
	std::vector<glm::quat> orientations(100);
	for(int i=0; i<100; i++){
 positions[i] = glm::vec3(rand()%20-10, rand()%20-10, rand()%20-10);
 orientations[i] = glm::quat(glm::vec3(rand()%360, rand()%360, rand()%360));
	}
 
 
 
	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
 
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
 
 
 // Compute the MVP matrix from keyboard and mouse input
 computeMatricesFromInputs();
 glm::mat4 ProjectionMatrix = getProjectionMatrix();
 glm::mat4 ViewMatrix = getViewMatrix();
 
 
 
 // PICKING IS DONE HERE
 // (Instead of picking each frame if the mouse button is down,
 // you should probably only check if the mouse button was just released)
 if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
 
 glm::vec3 ray_origin;
 glm::vec3 ray_direction;
 ScreenPosToWorldRay(
 1024/2, 768/2,
 1024, 768,
 ViewMatrix,
 ProjectionMatrix,
 ray_origin,
 ray_direction
 );
 
 //ray_direction = ray_direction*20.0f;
 
 message = "background";
 
 // Test each each Oriented Bounding Box (OBB).
 // A physics engine can be much smarter than this,
 // because it already has some spatial partitionning structure,
 // like Binary Space Partitionning Tree (BSP-Tree),
 // Bounding Volume Hierarchy (BVH) or other.
 for(int i=0; i<100; i++){
 
 float intersection_distance; // Output of TestRayOBBIntersection()
 glm::vec3 aabb_min(-1.0f, -1.0f, -1.0f);
 glm::vec3 aabb_max( 1.0f,  1.0f,  1.0f);
 
 // The ModelMatrix transforms :
 // - the mesh to its desired position and orientation
 // - but also the AABB (defined with aabb_min and aabb_max) into an OBB
 glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
 glm::mat4 TranslationMatrix = translate(mat4(), positions[i]);
 glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;
 
 
 if ( TestRayOBBIntersection(
 ray_origin,
 ray_direction,
 aabb_min,
 aabb_max,
 ModelMatrix,
 intersection_distance)
 ){
 std::ostringstream oss;
 oss << "mesh " << i;
 message = oss.str();
 break;
 }
 }
 
 
 }
 
 
 // Dark blue background
 glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
 // Re-clear the screen for real rendering
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
 
 // Use our shader
 glUseProgram(programID);
 
 glEnableVertexAttribArray(0);
 glEnableVertexAttribArray(1);
 glEnableVertexAttribArray(2);
 
 for(int i=0; i<100; i++){
 
 
 glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
 glm::mat4 TranslationMatrix = translate(mat4(), positions[i]);
 glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;
 
 glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
 
 // Send our transformation to the currently bound shader,
 // in the "MVP" uniform
 glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
 glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
 glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
 
 glm::vec3 lightPos = glm::vec3(4,4,4);
 glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
 
 // Bind our texture in Texture Unit 0
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, Texture);
 // Set our "myTextureSampler" sampler to use Texture Unit 0
 glUniform1i(TextureID, 0);
 
 // 1rst attribute buffer : vertices
 glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
 glVertexAttribPointer(
 0,                  // attribute
 3,                  // size
 GL_FLOAT,           // type
 GL_FALSE,           // normalized?
 0,                  // stride
 (void*)0            // array buffer offset
 );
 
 // 2nd attribute buffer : UVs
 glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
 glVertexAttribPointer(
 1,                                // attribute
 2,                                // size
 GL_FLOAT,                         // type
 GL_FALSE,                         // normalized?
 0,                                // stride
 (void*)0                          // array buffer offset
 );
 
 // 3rd attribute buffer : normals
 glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
 glVertexAttribPointer(
 2,                                // attribute
 3,                                // size
 GL_FLOAT,                         // type
 GL_FALSE,                         // normalized?
 0,                                // stride
 (void*)0                          // array buffer offset
 );
 
 // Index buffer
 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
 
 // Draw the triangles !
 glDrawElements(
 GL_TRIANGLES,      // mode
 indices.size(),    // count
 GL_UNSIGNED_SHORT,   // type
 (void*)0           // element array buffer offset
 );
 
 
 }
 
 glDisableVertexAttribArray(0);
 glDisableVertexAttribArray(1);
 glDisableVertexAttribArray(2);
 
 // Draw GUI
 TwDraw();
 
 // Swap buffers
 glfwSwapBuffers(window);
 glfwPollEvents();
 
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
 glfwWindowShouldClose(window) == 0 );
 
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
 
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
 
	return 0;
 }
 
 */
