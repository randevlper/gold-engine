#include "Transform.h"
#include "glm/gtx/matrix_transform_2d.hpp"

Transform::Transform()
{
	_position = { 0,0 };
	_scale = { 0,0 };
	_angle = 0;
	_parent = nullptr;
}

Transform::~Transform()
{

}

glm::vec2 Transform::getPosition()
{
	return _position;
}

void Transform::setPosition(glm::vec2 value)
{
	_position = value;
}

glm::vec2 Transform::getScale()
{
	return _scale;
}

void Transform::setScale(glm::vec2 value)
{
	_scale = value;
}

float Transform::getAngle()
{
	return _angle;
}

void Transform::setAngle(float value)
{
	_angle = value;
}

Transform* Transform::getParent()
{
	return _parent;
}

void Transform::setParent(Transform* value)
{
	_parent = value;
}

glm::mat3x3 Transform::getLocalMatrix()
{
	glm::mat3x3 retval;
	retval = glm::translate(retval, _position);
	retval = glm::scale(retval, _scale);
	retval = glm::rotate(retval, _angle);
	return  retval;
}

glm::mat3x3 Transform::getGlobalMatrix()
{
	if (_parent != nullptr) {
		return _parent->getGlobalMatrix() * getLocalMatrix();
	} else {
		return getLocalMatrix();
	}
}
