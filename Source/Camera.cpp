#include "Camera.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#define DEBUG false

const float MOVEMENT_SPEED = 0.5f;

Camera::Camera() :
_viewDirection(0.0f, 0.0f, -1.0f),
position(0.0f, 0.0f, 10.0f),
_up(0.0f, 1.0f, 0.0f),
_pivotPoint(0.0f, 0.0f, 0.0f)
{
}


void Camera::startDragging() {
    _cameraPressPos = position;
    _viewDirectionPressPoss = _viewDirection;
    _pivotPointPressPos = _pivotPoint;
}

void Camera::rotate(float dx, float dy)
{
    _strafeDirection = glm::cross(_viewDirection, _up);
    glm::mat4 rotator = glm::rotate(dx* 0.05f, _up) * glm::rotate(-dy * 0.05f, _strafeDirection);
    _viewDirection = glm::mat3(rotator) * _viewDirectionPressPoss;
    position = glm::length(_cameraPressPos)*-glm::normalize(_viewDirection);
}

void Camera::pan(float dx, float dy)
{
    position = _cameraPressPos + (dx)* glm::normalize(glm::cross(_viewDirection, _up)) + (dy)*_up;
    _pivotPoint = _pivotPointPressPos + (dx)* glm::normalize(glm::cross(_viewDirection, _up)) + (dy)*_up;
}

void Camera::zoom(float dy)
{
    position = _cameraPressPos + dy *_viewDirection;
}



glm::mat4 Camera::getWorldToViewMatrix() const
{
    return glm::lookAt(position, position + _viewDirection, _up);
}
