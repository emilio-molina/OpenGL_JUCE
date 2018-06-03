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
    _lastRotatePos = input;
}

Camera::Camera() :
_viewDirection(0.0f, 0.0f, -1.0f),
position(0.0f, 0.0f, 10.0f),
_up(0.0f, 1.0f, 0.0f),
_pivotPoint(0.0f, 0.0f, 0.0f),
pitch(0.0f),
yaw(0.0f),
roll(0.0f)
{
	setMousePressPos(glm::vec2(0.0f, 0.0f));
	rotate(glm::vec2(0.0f, 0.0f), 100,100);
}

void Camera::updateViewDirectionFromRotation() {
	_viewDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	_viewDirection.y = sin(glm::radians(pitch));
	_viewDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

	_viewDirection = glm::normalize(_viewDirection);
}


void Camera::rotate(const glm::vec2& pos, int width, int height)
{


	float dx = _lastRotatePos.x - pos.x;
		float dy = _lastRotatePos.y - pos.y;

		dx *= 0.2f;
		dy *= -0.2f;

    //std::cout << dy  << ", " << dx << std::endl;
    pitch -= dy;
    yaw -= dx;

	if(pitch > 89.0f)
		pitch =  89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	updateViewDirectionFromRotation();
	position = glm::length(_cameraPressPos)*-glm::normalize(_viewDirection);


	_lastRotatePos = pos;


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
    //position = _cameraPressPos + (dx)* glm::normalize(glm::cross(_viewDirection, _up)) + (dy)*_up;
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

    _pivotPoint = _pivotPointPressPos + dy *_viewDirection;

}



glm::mat4 Camera::getWorldToViewMatrix() const
{
    return glm::lookAt(position + _pivotPoint, position + _viewDirection + _pivotPoint, _up);
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

