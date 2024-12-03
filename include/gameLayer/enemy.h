#pragma once 
#include <gl2d/gl2d.h>

const float botSize = 150.f;

class Enemy
{
private:
	glm::uvec2 type = {};
	glm::vec2 position = {};
	glm::vec2 viewDirection = { 1, 0 };
	float speed = 1500.f;
	float turnSpeed = 3.f;


	float firedTime = 1.f;
	float fireTimeReset = 0.2;
	float fireRange = 1.5;
	float bulletSpeed = 2000;


public:
	Enemy();
	Enemy(glm::uvec2, glm::vec2, float, float, float, float);
	glm::vec2 getPos(); 
	glm::vec2 getView();
	void render(gl2d::Renderer2D& renderer, gl2d::Texture& sprites, gl2d::TextureAtlasPadding& atlas);
	bool update(float deltaTime, glm::vec2 playerPos);
};
