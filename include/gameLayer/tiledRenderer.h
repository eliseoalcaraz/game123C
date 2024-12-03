#pragma once 
#include <gl2d/gl2d.h>

class TiledRenderer
{
private:
	float backgroundSize;
	gl2d::Texture texture;
public:
	TiledRenderer();
	TiledRenderer(float,gl2d::Texture);
	void render(gl2d::Renderer2D& renderer);
};

void renderJet(gl2d::Renderer2D& renderer, glm::vec2 position, float size, gl2d::Texture texture, glm::vec4 uvs, glm::vec2 viewDirection);