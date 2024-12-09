
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
#include <vector>
#include <glui/glui.h>
#include <cstdio>

#include <enemy.h>
#include <tiledRenderer.h>
#include <bullets.h>
#include <GLFW/glfw3.h> //for window management

class GameData 
{
public:
	glm::vec2 playerPos = { 100, 100};
	std::vector<Bullets> bullets;
	std::vector<Enemy> enemies;

	float health = 1.f;
};

GameData data;


gl2d::Renderer2D renderer;

gl2d::Texture jetBodyTexture;
gl2d::TextureAtlasPadding jetAtlas;

gl2d::Texture jetPlayerTexture;

TiledRenderer tiledRenderer[2];
gl2d::Texture backgroundTexture[2];

gl2d::Texture bulletsTexture;
gl2d::TextureAtlasPadding bulletsAtlas;

gl2d::Texture healthBar;
gl2d::Texture health;

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


namespace platform
{
	void setWindowSize(int width, int height)
	{
		GLFWwindow* window = glfwGetCurrentContext(); // Get the current context
		if (window)
		{
			glfwSetWindowSize(window, width, height);
		}
		else
		{
			// Handle error (e.g., log a message)
			std::cerr << "Error: No active window context found.\n";
		}
	}
}


bool initGame()
{
	std::srand(std::time(0));
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();
	
	// Enforce a fixed window size
	const int FIXED_WINDOW_WIDTH = 1280;
	const int FIXED_WINDOW_HEIGHT = 720;
	platform::setWindowSize(FIXED_WINDOW_WIDTH, FIXED_WINDOW_HEIGHT);

	//game player sprite
	jetBodyTexture.loadFromFileWithPixelPadding
	(RESOURCES_PATH "spaceShip/stitchedFiles/spaceships.png", 128, true);
	jetAtlas = gl2d::TextureAtlasPadding(5, 2, jetBodyTexture.GetSize().x, jetBodyTexture.GetSize().y);
	jetPlayerTexture.loadFromFile(RESOURCES_PATH "jets/jet.png", true);

	//background
	backgroundTexture[0].loadFromFile(RESOURCES_PATH "background/sky_bg2.jpg", true);
	backgroundTexture[1].loadFromFile(RESOURCES_PATH "background/clouds_bg2.png", true);

	bulletsTexture.loadFromFileWithPixelPadding
	(RESOURCES_PATH "spaceShip/stitchedFiles/projectiles.png", 500, true);
	bulletsAtlas = gl2d::TextureAtlasPadding(3, 2, bulletsTexture.GetSize().x, bulletsTexture.GetSize().y);
	
	tiledRenderer[0] = TiledRenderer(5000, backgroundTexture[0]);
	tiledRenderer[1] = TiledRenderer(5000, backgroundTexture[1]);

	healthBar.loadFromFile(RESOURCES_PATH "healthBar.png", true);
	health.loadFromFile(RESOURCES_PATH "health.png", true);

	restartGame();
	
	return true;
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

	const float WORLD_BOUNDARY_X = 5000; // Adjust as per your game's world size
	const float WORLD_BOUNDARY_Y = 5000;

	// Restrict camera to world boundaries
	data.playerPos.x = glm::clamp(data.playerPos.x, 0.0f, WORLD_BOUNDARY_X);
	data.playerPos.y = glm::clamp(data.playerPos.y, 0.0f, WORLD_BOUNDARY_Y);

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

#pragma region handle bullets 
	const float jetSize = 180.f;

	if (platform::isLMousePressed())
	{
		Bullets b(data.playerPos, mouseDirection, false);

		data.bullets.push_back(b);
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
					data.enemies[e].damageLife(0.1);

					if (data.enemies[e].getLife() <= 0)
					{
						//kill enemy
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
				data.health -= 0.1;

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
		data.health += deltaTime * 0.01;
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
			Bullets b(data.enemies[i].getPos(), data.enemies[i].getView(), true);
			//todo speed
			data.bullets.push_back(b);
		}

	}

#pragma endregion

#pragma region render enemies

	for (auto& e : data.enemies)
	{
		e.render(renderer, jetBodyTexture, jetAtlas);
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

		glui::Box healthBox = glui::Box().xLeftPerc(0.05).yTopPerc(0.05).
			xDimensionPercentage(0.3).yAspectRatio(1.f / 8.f);

		renderer.renderRectangle(healthBox, healthBar);

		glm::vec4 newRect = healthBox();
		newRect.z *= data.health;

		glm::vec4 textCoords = { 0,1,1,0 };
		textCoords.z *= data.health;

		renderer.renderRectangle(newRect, health, Colors_White, {}, {},
			textCoords);


	}
	renderer.popCamera();

	renderer.flush();
	//ImGui::ShowDemoWindow();

	ImGui::Begin("debug");
	ImGui::Text("Bullets 1 count: %d", (int)data.bullets.size());
	ImGui::Text("Enemies count: %d", (int)data.enemies.size());

	if (ImGui::Button("Spawn enemy"))
	{
		glm::uvec2 shipTypes[] = { {0,0}, {0,1}, {2,0}, {3, 1} };
		float speed = 800 + rand() % 1000;
		float turnSpeed = 2.2f + (rand() & 1000) / 500.f;
		glm::uvec2 type = shipTypes[rand() % 4];
		float fireRange = 1.5 + (rand() % 1000) / 2000.f;
		float fireTimeReset = 0.1 + (rand() % 1000) / 500;

		Enemy e(type, data.playerPos, speed, turnSpeed, fireRange, fireTimeReset);
		data.enemies.push_back(e);
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
