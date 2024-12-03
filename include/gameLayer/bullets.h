#pragma once
#include <gl2d/gl2d.h>

class Bullets {
private:
	glm::vec2 position = {};
	glm::vec2 fireDirection = {};	

public:
	Bullets();
	Bullets(glm::vec2, glm::vec2);
	glm::vec2 getPos();
	void render(gl2d::Renderer2D& renderer,
		gl2d::Texture bulletsTexture, gl2d::TextureAtlasPadding bulletsAtlas
	);

	void update(float deltaTime);
};	