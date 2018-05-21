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
    glm::vec3 _strafeDirection;
    glm::vec2 _mousePressPos;
    glm::vec3 _cameraPressPos;
    glm::vec3 _viewDirectionPressPoss;
    glm::vec3 _pivotPoint;
    glm::vec3 _pivotPointPressPos;
    
public:
    Camera();
    glm::vec3 position;
    int _width;
    int _height;
    glm::mat4 getWorldToViewMatrix() const;
    void rotate(float deltaX, float deltaY);
    void pan(float deltaX, float deltaY);
    void zoomByDragging(float deltaZoom);
    void zoomByWheel(float deltaZoom);
    void startDragging();
};

#endif

