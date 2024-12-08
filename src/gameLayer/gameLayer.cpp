#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>
#include <glui/glui.h>
#include <cstdio>
#include <raudio.h>
#include <cmath>
#include <string>

#include <enemy.h>
#include <tiledRenderer.h>
#include <bullets.h>
#include <load.h>

#include <vector>
#include <queue>


class GameData 
{
public:
	glm::vec2 playerPos = { 100, 100};
	std::vector<Bullets> bullets;
	std::vector<Enemy> enemies;
	std::vector<LoadBullet> loads;

	std::queue<LoadBullet> jetLoad;

	float health = 1.f;
	float spawnTimeEnemy = 3;
	int points = 0;
};

GameData data;

gl2d::Renderer2D renderer;

gl2d::Texture jetBodyTexture;
gl2d::TextureAtlasPadding jetAtlas;

gl2d::Texture jetPlayerTexture;
gl2d::Texture botTexture[4];

TiledRenderer tiledRenderer[2];
gl2d::Texture backgroundTexture[2];

gl2d::Texture bulletsTexture;
gl2d::TextureAtlasPadding bulletsAtlas;

gl2d::Texture reloadBullet[3];

gl2d::Texture healthBar;
gl2d::Texture health;

gl2d::Font font;

Sound shootSound;

bool intersectBullet(glm::vec2 bulletPos, glm::vec2 shipPos, float shipSize)
{
	return glm::distance(bulletPos, shipPos) <= shipSize;
}

void restartGame()
{
	data = {};
	renderer.currentCamera.follow(data.playerPos
		, 550, 0, 0, renderer.windowW, renderer.windowH);
}

bool initGame()
{
	std::srand(std::time(0));
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();
	
	//game player sprite
	jetBodyTexture.loadFromFileWithPixelPadding
	(RESOURCES_PATH "spaceShip/stitchedFiles/spaceships.png", 128, true);
	jetAtlas = gl2d::TextureAtlasPadding(5, 2, jetBodyTexture.GetSize().x, jetBodyTexture.GetSize().y);
	jetPlayerTexture.loadFromFile(RESOURCES_PATH "jets/jet.png", true);
	botTexture[0].loadFromFile(RESOURCES_PATH "jets/1.png", true);
	botTexture[1].loadFromFile(RESOURCES_PATH "jets/2.png", true);
	botTexture[2].loadFromFile(RESOURCES_PATH "jets/3.png", true);
	botTexture[3].loadFromFile(RESOURCES_PATH "jets/4.png", true);

	//background
	backgroundTexture[0].loadFromFile(RESOURCES_PATH "background/sky_bg2.jpg", true);
	backgroundTexture[1].loadFromFile(RESOURCES_PATH "background/clouds_bg2.png", true);

	bulletsTexture.loadFromFileWithPixelPadding
	(RESOURCES_PATH "spaceShip/stitchedFiles/projectiles.png", 500, true);
	bulletsAtlas = gl2d::TextureAtlasPadding(3, 2, bulletsTexture.GetSize().x, bulletsTexture.GetSize().y);
	
	reloadBullet[0].loadFromFile(RESOURCES_PATH "bullets/reload/1.png", true);
	reloadBullet[1].loadFromFile(RESOURCES_PATH "bullets/reload/2.png", true);
	reloadBullet[2].loadFromFile(RESOURCES_PATH "bullets/reload/3.png", true);

	tiledRenderer[0] = TiledRenderer(5000, backgroundTexture[0]);
	tiledRenderer[1] = TiledRenderer(5000, backgroundTexture[1]);

	healthBar.loadFromFile(RESOURCES_PATH "healthBar.png", true);
	health.loadFromFile(RESOURCES_PATH "health.png", true);

	shootSound = LoadSound(RESOURCES_PATH "shootSfx.flac");
	SetSoundVolume(shootSound, 0.1);

	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");

	restartGame();
	
	return true;
}

void spawnEnemy()
{
	int type;
	if (data.points < 10)
		type = 0;
	else if (data.points >= 10 && data.points < 30)
		type = rand() % 2;
	else if (data.points >= 30 && data.points < 60)
		type = rand() % 3;
	else
		type = rand() % 4;

	std::cout << type << std::endl;

	glm::vec2 offset(2000, 0);
	offset = glm::vec2(glm::vec4(offset, 0, 1) * glm::rotate(glm::mat4(1.f), glm::radians((float)(rand() % 360)), glm::vec3(0, 0, 1)));

	float speed = 800 + rand() % 1000;
	float turnSpeed = 2.2f + (rand() & 1000) / 500.f;
	float fireRange = 1.5 + (rand() % 1000) / 2000.f;
	float fireTimeReset = 0.1 + (rand() % 1000) / 500;
	glm::vec2 position = data.playerPos + offset;

	Enemy e(type, position, speed, turnSpeed, fireRange, fireTimeReset);
	data.enemies.push_back(e);
}

void spawnLoads() {

	if (data.loads.size() < 15) {
		int typeBullet = rand() % 3;
		glm::vec2 offset(1500, 0);
		glm::vec2 offsetBullet = glm::vec2(glm::vec4(offset, 0, 1) * glm::rotate(glm::mat4(1.f), glm::radians((float)(rand() % 360)), glm::vec3(0, 0, 1)));
		glm::vec2 posBullet = data.playerPos + offsetBullet;
		int load = 10;

		LoadBullet l(posBullet, typeBullet, load);

		data.loads.push_back(l);
	}
	
}

std::string level(int points) {
	if (points < 10)
		return "Beginner";
	else if (points >= 10 && points < 60)
		return "Average";
	else
		return "Master";
		
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getFrameBufferSizeX(); //window w
	h = platform::getFrameBufferSizeY(); //window h
	
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); //clear screen

	renderer.updateWindowMetrics(w, h);
	const float jetSize = 180.f;
#pragma endregion

#pragma region movement on player 

	glm::vec2 move = {};

	if (
		platform::isButtonHeld(platform::Button::W) ||
		platform::isButtonHeld(platform::Button::Up)
		)
	{
		move.y = -1;
	}
	if (
		platform::isButtonHeld(platform::Button::S) ||
		platform::isButtonHeld(platform::Button::Down)
		)
	{
		move.y = 1;
	}
	if (
		platform::isButtonHeld(platform::Button::A) ||
		platform::isButtonHeld(platform::Button::Left)
		)
	{
		move.x = -1;
	}
	if (
		platform::isButtonHeld(platform::Button::D) ||
		platform::isButtonHeld(platform::Button::Right)
		)
	{
		move.x = 1;
	}

	if (move.x != 0 || move.y != 0)
	{
		move = glm::normalize(move);
		move *= deltaTime * 1500; //200 pixels per seccond
		data.playerPos += move;
	}

#pragma endregion

#pragma region camera follow

	renderer.currentCamera.follow(data.playerPos, deltaTime * 550, 1, 150, w, h);

#pragma endregion 

#pragma region render background

	for(int i = 0; i < 2; i++)
		tiledRenderer[i].render(renderer);

#pragma endregion

#pragma region mouse pos

	glm::vec2 mousePos = platform::getRelMousePosition();
	glm::vec2 screenCenter(w / 2.f, h / 2.f);

	glm::vec2 mouseDirection = mousePos - screenCenter;

	if (glm::length(mouseDirection) == 0.f)
	{
		mouseDirection = { 1,0 };
	}
	else
	{
		mouseDirection = normalize(mouseDirection);
	}

	float jetAngle = atan2(mouseDirection.y, -mouseDirection.x);

#pragma endregion

#pragma region render loads bullets

	spawnLoads();

	for (int i = 0; i < data.loads.size(); i++) {

		if (glm::distance(data.playerPos, data.loads[i].getPos()) > 4000.f)
		{
			data.loads.erase(data.loads.begin() + i);
			i--;
			continue;
		}
		int type = data.loads[i].getType();

		renderer.renderRectangle({ data.loads[i].getPos(), 100.f, 100.f}, reloadBullet[type], Colors_White, {}, {});

	}

	for (int i = 0; i < data.loads.size(); i++) {
		
		if (intersectBullet(data.loads[i].getPos(), data.playerPos, jetSize)) {
			LoadBullet l(data.loads[i].getPos(), data.loads[i].getType(), data.loads[i].getLoad());

			data.jetLoad.push(l);
			data.loads.erase(data.loads.begin() + i);
			i--;
			continue;
		}
	}

#pragma endregion

#pragma region handle bullets 

	if (platform::isLMousePressed() && !(data.jetLoad.empty()) )
	{
		if (data.jetLoad.front().canLoadBullet()) {
			Bullets b(data.playerPos, mouseDirection, false, data.jetLoad.front().getDamage());
			data.bullets.push_back(b);
			PlaySound(shootSound);
		}
		else
			data.jetLoad.pop();
		
	}


	for (int i = 0; i < data.bullets.size(); i++)
	{

		if (glm::distance(data.bullets[i].getPos(), data.playerPos) > 5'000)
		{
			data.bullets.erase(data.bullets.begin() + i);
			i--;
			continue;
		}

		if (!data.bullets[i].isEnemy)
		{
			bool breakBothLoops = false;
			for (int e = 0; e < data.enemies.size(); e++)
			{

				if (intersectBullet(data.bullets[i].getPos(), data.enemies[e].getPos(),
					jetSize))
				{
					data.enemies[e].damageLife(data.bullets[i].getDamage());

					if (data.enemies[e].getLife() <= 0)
					{
						//kill enemy
						data.points += (data.enemies[e].getType() + 1);
						data.enemies.erase(data.enemies.begin() + e);
					}

					data.bullets.erase(data.bullets.begin() + i);
					i--;
					breakBothLoops = true;
					continue;
				}

			}

			if (breakBothLoops)
			{
				continue;
			}
		}
		else
		{
			if (intersectBullet(data.bullets[i].getPos(), data.playerPos,
				jetSize))
			{
				data.health -= data.bullets[i].getDamage();

				data.bullets.erase(data.bullets.begin() + i);
				i--;
				continue;
			}

		}

		data.bullets[i].update(deltaTime);

	}

	if (data.health <= 0)
	{
		//kill player
		restartGame();
	}
	else
	{
		data.health += deltaTime * 0.05;
		data.health = glm::clamp(data.health, 0.f, 1.f);
	}

#pragma endregion 

#pragma region render bullets
	for (auto& b : data.bullets)
	{
		b.render(renderer, bulletsTexture, bulletsAtlas);
	}

#pragma endregion

#pragma region handle bullets enemies

	if (data.enemies.size() < 10)
	{
		data.spawnTimeEnemy -= deltaTime;

		if (data.spawnTimeEnemy  < 0)
		{
			data.spawnTimeEnemy = rand() % 6 + 1;

			spawnEnemy();
			if (rand() % 3 == 0)
			{
				spawnEnemy();
				spawnEnemy();
			}

		}

	}

	for (int i = 0; i < data.enemies.size(); i++)
	{
		if (glm::distance(data.playerPos, data.enemies[i].getPos()) > 4000.f)
		{
			//dispawn enemy
			data.enemies.erase(data.enemies.begin() + i);
			i--;
			continue;
		}

		if (data.enemies[i].update(deltaTime, data.playerPos))
		{
			Bullets b(data.enemies[i].getPos(), data.enemies[i].getView(), true, data.enemies[i].getDamage());
			//todo speed
			data.bullets.push_back(b);
			if (!IsSoundPlaying(shootSound)) PlaySound(shootSound);
		}

	}

#pragma endregion

#pragma region render enemies

	for (auto& e : data.enemies)
	{
		int type = e.getType();
		if (!(type >= 0 && type <= 3))
			type = 0;
		e.render(renderer, botTexture[type]);
	}

#pragma endregion

#pragma region render jet

	renderer.renderRectangle({ data.playerPos - glm::vec2(jetSize / 2,jetSize / 2)
		, jetSize,jetSize }, jetPlayerTexture,
		Colors_White, {}, glm::degrees(jetAngle) + 90.f);

#pragma endregion
	
	

	renderer.pushCamera();
	{

		glui::Frame f({ 0,0, w, h });

		glui::Box healthBox = glui::Box().xLeftPerc(0.65).yTopPerc(0.05).
			xDimensionPercentage(0.3).yAspectRatio(1.f / 8.f);

		renderer.renderRectangle(healthBox, healthBar);

		glm::vec4 newRect = healthBox();
		newRect.z *= data.health;

		glm::vec4 textCoords = { 0,1,1,0 };
		textCoords.z *= data.health;

		renderer.renderRectangle(newRect, health, Colors_White, {}, {},
			textCoords);


		std::string currentPoints = "Score: " + std::to_string(data.points);
		const char* points = currentPoints.c_str();
		std::string currentLevel = level(data.points);
		const char* myLevel = currentLevel.c_str();

		renderer.renderText(glm::vec2{ 150, 50 }, myLevel, font, Colors_Black, (1.0F), (4.0F), (3.0F), true, {});
		renderer.renderText(glm::vec2{ 500, 50 }, points, font, Colors_Black, (1.0F), (4.0F), (3.0F), true, {});

	}
	renderer.popCamera();

	renderer.flush();
	//ImGui::ShowDemoWindow();

	ImGui::Begin("debug");
	ImGui::Text("Bullets 1 count: %d", (int)data.bullets.size());
	ImGui::Text("Enemies count: %d", (int)data.enemies.size());
	ImGui::Text("Points : %d", (int)data.points);

	if (ImGui::Button("Spawn enemy"))
	{
		spawnEnemy();
	}

	if (ImGui::Button("Reset game"))
	{
		restartGame();
	}

	ImGui::SliderFloat("Player Health", &data.health, 0, 1);

	ImGui::End();

	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{



}
