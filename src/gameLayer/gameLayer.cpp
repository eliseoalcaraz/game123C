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
#include <tiledRenderer.h>
#include <bullets.h>
#include <vector>

class GameData 
{
public:
	glm::vec2 player1Pos = { 400,450 };
	glm::vec2 player1Angle = { 1, 0 };
	glm::vec2 player2Pos = { 1350,450 };
	glm::vec2 player2Angle = { -1, 0 };

	std::vector<Bullets> bullets1;
	std::vector<Bullets> bullets2;
};

GameData data;
TiledRenderer tiledRenderer;

gl2d::Renderer2D renderer;

gl2d::Texture human1BodyTexture;
gl2d::Texture human2BodyTexture;
gl2d::Texture backgroundTexture;

gl2d::Texture bulletsTexture;
gl2d::TextureAtlasPadding bulletsAtlas;

bool initGame()
{
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	human1BodyTexture.loadFromFile(RESOURCES_PATH "jet.png", true); //replace this sprite if naa na;
	human2BodyTexture.loadFromFile(RESOURCES_PATH "jet.png", true); 
	backgroundTexture.loadFromFile(RESOURCES_PATH "tempBackground.png", true);

	bulletsTexture.loadFromFileWithPixelPadding
	(RESOURCES_PATH "spaceShip/stitchedFiles/projectiles.png", 500, true);
	bulletsAtlas = gl2d::TextureAtlasPadding(3, 2, bulletsTexture.GetSize().x, bulletsTexture.GetSize().y);
	
	tiledRenderer = TiledRenderer(5000, backgroundTexture);

	
	return true;
}


//hey
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

#pragma region movement on player 1

	glm::vec2 move1 = {};

	if (platform::isButtonHeld(platform::Button::W))
		move1.y = -1;
	if (platform::isButtonHeld(platform::Button::S))
		move1.y = 1;
	if (platform::isButtonHeld(platform::Button::A))
		move1.x = -1;
	if (platform::isButtonHeld(platform::Button::D))
		move1.x = 1;

	if (move1.x != 0 || move1.y != 0)
	{
		move1 = glm::normalize(move1);
		data.player1Angle = move1;
		move1 *= deltaTime * 500;
		data.player1Pos += move1;
	}

	float jet1Angle = atan2(-data.player1Angle.x, -data.player1Angle.y);


#pragma endregion

#pragma region movement on player 2

	glm::vec2 move2 = {};

	if (platform::isButtonHeld(platform::Button::Up))
		move2.y = -1;
	if (platform::isButtonHeld(platform::Button::Down))
		move2.y = 1;
	if (platform::isButtonHeld(platform::Button::Left))
		move2.x = -1;
	if (platform::isButtonHeld(platform::Button::Right))
		move2.x = 1;

	if (move2.x != 0 || move2.y != 0)
	{
		move2 = glm::normalize(move2);
		data.player2Angle = move2;
		move2 *= deltaTime * 500;
		data.player2Pos += move2;
	}
	
	float jet2Angle = atan2(-data.player2Angle.x, -data.player2Angle.y);

#pragma endregion

#pragma region camera follow

	glm::vec2 midpoint = {
		(data.player1Pos.x + data.player2Pos.x) / 2,
		(data.player1Pos.y + data.player2Pos.y) / 2
	};

	renderer.currentCamera.follow(midpoint, deltaTime * 450, 10, 50, w, h);

#pragma endregion 

#pragma region handle bullets 1

	if (platform::isButtonPressedOn(platform::Button::G)) {
		Bullets b(data.player1Pos, data.player1Angle);
		data.bullets1.push_back(b);
	}

	for (int i = 0; i < data.bullets1.size(); i++)
	{

		if (glm::distance(data.bullets1[i].getPos(), data.player1Pos) > 5'000)
		{
			data.bullets1.erase(data.bullets1.begin() + i);
			i--;
			continue;
		}
		data.bullets1[i].update(deltaTime);
	}

#pragma endregion 

#pragma region handle bullets 2

	if (platform::isButtonPressedOn(platform::Button::L)) {
		Bullets b(data.player2Pos, data.player2Angle);
		data.bullets2.push_back(b);
	}

	for (int i = 0; i < data.bullets2.size(); i++)
	{

		if (glm::distance(data.bullets2[i].getPos(), data.player2Pos) > 5'000)
		{
			data.bullets2.erase(data.bullets2.begin() + i);
			i--;
			continue;
		}
		data.bullets2[i].update(deltaTime);
	}

#pragma endregion 

#pragma region render background

	tiledRenderer.render(renderer);

#pragma endregion

#pragma region render bullets
	for (auto& b : data.bullets1)
	{
		b.render(renderer, bulletsTexture, bulletsAtlas);
	}
	for (auto& b : data.bullets2)
	{
		b.render(renderer, bulletsTexture, bulletsAtlas);
	}

#pragma endregion



	renderer.renderRectangle({ data.player1Pos, 170, 170 }, human1BodyTexture, Colors_White, {}, glm::degrees(jet1Angle));
	renderer.renderRectangle({ data.player2Pos, 170, 170 }, human2BodyTexture, Colors_White, {}, glm::degrees(jet2Angle));

	renderer.flush();


	//ImGui::ShowDemoWindow();

	ImGui::Begin("debug");
	ImGui::Text("Bullets 1 count: %d", (int)data.bullets1.size());
	ImGui::Text("Bullets 2 count: %d", (int)data.bullets2.size());
	ImGui::End();


	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{



}
