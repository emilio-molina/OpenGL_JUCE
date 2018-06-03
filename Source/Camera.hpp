//
//  Camera.cpp
//  OpenGL_JUCE - App
//
//  Created by Chebonenko Vitali on 3/6/18.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>


class Camera{
    

    glm::vec3 _viewDirection;
    const glm::vec3  _up;
    glm::vec2 _oldMousePosition;
    glm::vec3 _strafeDirection;
    glm::vec2 _mousePressPos;
    glm::vec3 _cameraPressPos;
    glm::vec3 _viewDirectionPressPoss;
    glm::vec3 _pivotPoint;
    glm::vec3 _pivotPointPressPos;
    glm::vec2 _lastRotatePos;

    float pitch, yaw, roll;

    void updateViewDirectionFromRotation();
    
public:
    Camera();
    glm::vec3 position;
    glm::mat4 getWorldToViewMatrix() const;
    void rotate(const glm::vec2& newMousePosition, int width, int heigh);
    void pan(const glm::vec2& newMousePosition, int width, int heigh);
    void zoom(const glm::vec2& newMousePosition, int width, int heigh);
    
    void moveForward();
    void moveBackward();
    void strafeLeft();
    void strafeRight();
    void moveUp();
    void moveDown();
    
    void setMousePressPos(const glm::vec2& input);
    
};

#endif

