#include "Camera.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#define DEBUG false

const float MOVEMENT_SPEED = 0.5f;



void Camera::setMousePressPos(const glm::vec2& input)
{
    _cameraPressPos = position;
    _mousePressPos = input;
    _viewDirectionPressPoss = _viewDirection;
    _pivotPointPressPos = _pivotPoint;
}

Camera::Camera() :
_viewDirection(0.0f, 0.0f, -1.0f),
position(0.0f, 0.0f, 5.2f),
_up(0.0f, 1.0f, 0.0f),
_pivotPoint(0.0f, 0.0f, 0.0f)
{
}


void Camera::rotate(const glm::vec2& pos, int width, int height)
{
    
    float sX = (2.0f * _mousePressPos.x) / (float)width - 1.0f;
    float sY = 1.0f - (2.0f * _mousePressPos.y) / (float)height;
    
    float xPoint = (2.0f * pos.x) / (float)width - 1.0f;
    float yPoint = 1.0f - (2.0f * pos.y) / (float)height;
    
    float dx, dy;
    dx = (float)(sX - xPoint) * 10.f;
    dy = (float)(sY - yPoint) * 20.f;
    
    if (glm::length(dx) > 50.0f)
    {
        _oldMousePosition = pos;
        return;
    }
    
    _strafeDirection = glm::cross(_viewDirection, _up);
    glm::mat4 rotator = glm::rotate(dx* 0.05f, _up) * glm::rotate(-dy * 0.05f, _strafeDirection);
    _viewDirection = glm::mat3(rotator) * _viewDirectionPressPoss;
    position = glm::length(_cameraPressPos)*-glm::normalize(_viewDirection);
}

void Camera::pan(const glm::vec2& pos, int width, int height)
{
    
    float sX = (2.0f * _mousePressPos.x) / width - 1.0f;
    float sY = 1.0f - (2.0f * _mousePressPos.y) / height;
    
    float xPoint = (2.0f * pos.x) / width - 1.0f;
    float yPoint = 1.0f - (2.0f * pos.y) / height;
    
    float dx, dy;
    dx = (float)(sX - xPoint) * 4;
    dy = (float)(sY - yPoint) * 4;
    position = _cameraPressPos + (dx)* glm::normalize(glm::cross(_viewDirection, _up)) + (dy)*_up;
    _pivotPoint = _pivotPointPressPos + (dx)* glm::normalize(glm::cross(_viewDirection, _up)) + (dy)*_up;
}

void Camera::zoom(const glm::vec2& pos, int width, int height)
{
    
    float sX = (2.0f * _mousePressPos.x) / width - 1.0f;
    float sY = 1.0f - (2.0f * _mousePressPos.y) / height;
    
    float xPoint = (2.0f * pos.x) / width - 1.0f;
    float yPoint = 1.0f - (2.0f * pos.y) / height;
    
    float dx, dy;
    dx = (float)(sX - xPoint) * 10;
    dy = (float)(sY - yPoint) * 10;
    
    position = _cameraPressPos + dy *_viewDirection;
    
}



glm::mat4 Camera::getWorldToViewMatrix() const
{
    return glm::lookAt(position, position + _viewDirection, _up);
}


void Camera::moveForward()
{
    position += MOVEMENT_SPEED * _viewDirection;
    if (DEBUG)
        std::cout << "move forward" << "\n";
}

void Camera::moveBackward()
{
    position += -MOVEMENT_SPEED * _viewDirection;
    if (DEBUG)
        std::cout << "move backward" << "\n";
}
void Camera::strafeLeft()
{
    position += - MOVEMENT_SPEED * _strafeDirection;
    if (DEBUG)
        std::cout << "strafe Left" << "\n";
}
void Camera::strafeRight()
{
    position += MOVEMENT_SPEED * _strafeDirection;
    if (DEBUG)
        std::cout << "strafe Right" << "\n";
}
void Camera::moveUp()
{
    position += MOVEMENT_SPEED * _up;
    if (DEBUG)
        std::cout << "move Up" << "\n";
}
void Camera::moveDown()
{
    position += -MOVEMENT_SPEED * _up;
    if (DEBUG)
        std::cout << "move Down" << "\n";
}
