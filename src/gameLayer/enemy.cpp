#include <enemy.h>
#include <tiledRenderer.h>

Enemy::Enemy() {
	viewDirection = { 1, 0 };
}

Enemy::Enemy(glm::uvec2 t, glm::vec2 pos, float s, float tSpeed, float fR, float fT) {
	type = t;
	position = pos;
	speed = s;
	turnSpeed = tSpeed;
	fireRange = fR;
	fireTimeReset = fT;
}

void Enemy::render(gl2d::Renderer2D& renderer, gl2d::Texture& sprites, gl2d::TextureAtlasPadding& atlas) {
	renderJet(renderer, position, botSize, sprites, atlas.get(type.x, type.y), viewDirection);
}

glm::vec2 Enemy::getPos() {
	return position;
}

glm::vec2 Enemy::getView() {
	return viewDirection;
}

bool Enemy::update(float deltaTime, glm::vec2 playerPos)
{

	glm::vec2 directionToPlayer = playerPos - position;
	if (glm::length(directionToPlayer) == 0) { directionToPlayer = { 1,0 }; }
	else { directionToPlayer = glm::normalize(directionToPlayer); }

	glm::vec2 newDirection = {};

	bool shoot = (glm::length(directionToPlayer + viewDirection) >= fireRange);

	if (shoot)
	{
		if (firedTime <= 0.f)
		{
			firedTime = fireTimeReset;
		}
		else
		{
			shoot = 0;
		}
	}

	firedTime -= deltaTime;
	if (firedTime < 0) { firedTime = 0.f; }


	if (glm::length(directionToPlayer + viewDirection) <= 0.2)
	{
		newDirection = glm::vec2(directionToPlayer.y, -directionToPlayer.x);
		if (rand() % 2)
		{
			newDirection = glm::vec2(directionToPlayer.y, -directionToPlayer.x);
		}
		else
		{
			newDirection = glm::vec2(-directionToPlayer.y, directionToPlayer.x);
		}

	}
	else
	{
		newDirection =
			deltaTime * turnSpeed * directionToPlayer + viewDirection;
	}
	float length = glm::length(newDirection);
	viewDirection = glm::normalize(newDirection);
	length = glm::clamp(length, 0.1f, 3.f);

	position += viewDirection * deltaTime * speed * length;

	return shoot;
}

void Enemy::damageLife(float damage) {
	life -= damage;
}

float Enemy::getLife() {
	return life;
}