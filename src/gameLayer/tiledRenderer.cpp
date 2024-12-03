#include <tiledRenderer.h>

TiledRenderer::TiledRenderer() {
	this->backgroundSize = 5000;
}

TiledRenderer::TiledRenderer(float size, gl2d::Texture tex) {
	this->backgroundSize = size;
	this->texture = tex;
}

void TiledRenderer::render(gl2d::Renderer2D& renderer) {
	auto viewRect = renderer.getViewRect();
	//this allows us to see what we can see
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			int posX = x + int(viewRect.x / backgroundSize);
			int posY = y + int(viewRect.y / backgroundSize);

			renderer.renderRectangle(
				glm::vec4{ posX, posY, 1, 1 } *backgroundSize, texture);
		}
	}
	renderer.renderRectangle({ 0, 0, backgroundSize, backgroundSize }, texture);
}

void renderJet(gl2d::Renderer2D& renderer, glm::vec2 position, float size, gl2d::Texture texture, glm::vec4 uvs, glm::vec2 viewDirection){

	float jetAngle = atan2(viewDirection.y, -viewDirection.x);

	renderer.renderRectangle({ position - glm::vec2(size / 2, size / 2)
	, size,size }, texture,
		Colors_White, {}, glm::degrees(jetAngle) + 90.f,
		uvs);
}


