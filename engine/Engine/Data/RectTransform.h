#pragma once
#include <glm/vec2.hpp>
#include "Transform.h"
#define RECT_TRANSFORM_SIZE 4

class RectTransform
{
public:
	
	
	//Four corners
	RectTransform();
	~RectTransform();

	void setSize(glm::ivec2 value);

	void setPosition(glm::ivec2 value);
	glm::vec2 getPosition();
	
	void setRotation(float value);
	void setScale(glm::vec2 value);

	

	glm::ivec2 getSize() {
		return _size;
	}

	//Counter clockwise starting at bottom left
	glm::vec2* getScreenCorners();
	glm::vec2* getWindowCorners();

	nlohmann::json get_json();
	void from_json(const nlohmann::json& j);

private:

	glm::ivec2 _size;

	Transform _root;
	Transform _transforms[4];

	glm::vec2 _corners[4];
	glm::vec2 _windowCorners[4];
};