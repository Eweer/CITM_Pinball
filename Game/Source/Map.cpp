#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Audio.h"
#include "EntityManager.h"
#include "Fonts.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>
#include <iomanip>
#include <sstream>

#include "SDL_image/include/SDL_image.h"

#include "PugiXml/src/pugixml.hpp"

#include <iostream>

Map::Map() : Module()
{
	name.Create("map");
}

// Destructor
Map::~Map() = default;

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");

	texturePath = config.child("mapfolder").attribute("texturepath").as_string();
	std::string audioPath = config.child("mapfolder").attribute("audiopath").as_string();
	musicPath = audioPath + config.child("mapfolder").attribute("musicfolder").as_string();

	fontsPath = texturePath + config.child("mapfolder").attribute("fontsfolder").as_string();

	return true;
}

bool Map::Start()
{
	std::string fontWhiteFile = fontsPath + "font_white.png";
	std::string fontOrangeFile = fontsPath + "font_orange.png";

	std::string characters = " .!'0123456789ABCEFGHILNPRSTUabcdefghiklnoprstuvy   ";

	fontWhite = app->fonts->Load(fontWhiteFile.c_str(), characters.c_str(), 2);
	fontOrange = app->fonts->Load(fontOrangeFile.c_str(), characters.c_str(), 2);

	return true;
}
bool Map::PostUpdate()
{
	DrawUI();
	return true;
}
void Map::Draw()
{
	app->render->DrawTexture(backgroundImage, 0, 0);
	app->render->DrawTexture(boardImage, 0, 0);
	
}

// Called before quitting
bool Map::CleanUp()
{
	LOG("Unloading map");

	if(boardImage) app->tex->UnLoad(boardImage);

	return true;
}

// Load new map
bool Map::Load()
{
	uint aux = app->GetLevelNumber();

	auto levelFilePath = texturePath + "level_" + std::to_string(aux) + "/";
	auto imageFolder = levelFilePath + "board.png";
	boardImage = app->tex->Load(imageFolder.c_str());
	
	imageFolder = levelFilePath + "background.png";
	backgroundImage = app->tex->Load(imageFolder.c_str());

	std::string musicFileName = musicPath + "level_" + std::to_string(aux) + ".ogg";
	backgroundMusic = app->audio->PlayMusic(musicFileName.c_str());

	return true;
}

void Map::DrawUI() const
{
	DrawScores(560, 95);
	DrawFPS(5, 25);
	DrawGravity(5, 65);
}

void Map::DrawGravity(int x, int y) const
{

	b2Vec2 gravity = app->physics->GetWorldGravity();
	iPoint pos(x, y);

	std::stringstream streamX;
	streamX << std::fixed << std::setprecision(1) << gravity.x;
	std::string gravX = streamX.str();
	std::stringstream streamY;
	streamY << std::fixed << std::setprecision(1) << gravity.y;
	std::string gravY = streamY.str();

	app->fonts->Blit(pos.x, pos.y, fontWhite, "Gravity");

	pos.y += 20;

	app->fonts->Blit(pos.x, pos.y, fontWhite, "'");

	pos.x += 10;

	if (gravity.x < 0)
	{
		app->fonts->Blit(pos.x, pos.y, fontOrange, "!");
		pos.x += 5;
	}
	if (abs(gravity.x) > 9) pos.x += 5;

	app->fonts->Blit(pos.x, pos.y, fontOrange, gravX.c_str());


	if (gravity.y < 0) pos.x += 10;
	if (abs(gravity.y) > 9) pos.x += 10;

/*	app->fonts->Blit(pos.x + 50, pos.y, fontWhite, ".");

	if (gravity.y < 0)
	{
		app->fonts->Blit(pos.x + pos.y, pos.y, fontOrange, "!");
		pos.x += 5;
	}
	if (abs(gravity.y) > 9) pos.x += 5;

	app->fonts->Blit(pos.x + pos.y, pos.y, fontOrange, gravY.c_str());

	if (gravity.y < 0) pos.x += 15;
	if (abs(gravity.y) > 9) pos.x += 15;

	app->fonts->Blit(pos.x + 110, pos.y, fontWhite, "'"); */
}

void Map::DrawFPS(int x, int y) const
{
	std::string currentFPS = std::to_string(app->render->GetCurrentFPS());
	app->fonts->Blit(5, 25, fontWhite, "CURRENT FPS ");
	app->fonts->Blit(185, 25, fontWhite, currentFPS.c_str());
	std::string targetFPS = std::to_string(app->render->GetTargetFPS());
	app->fonts->Blit(5, 45, fontOrange, "TARGET  FPS ");
	app->fonts->Blit(185, 45, fontOrange, targetFPS.c_str());
}

void Map::DrawScores(int x, int y) const
{
	std::string score = std::to_string(app->entityManager->GetScore());
	std::pair<uint, uint> scoreList = app->entityManager->GetScoreList();
	std::string highScore = std::to_string(scoreList.first);
	std::string prevScore = std::to_string(scoreList.second);
	app->fonts->Blit(560, 95, fontWhite, "SC0RE ", 4);
	app->fonts->Blit(560, 115, fontOrange, "HIGH", 4);
	app->fonts->Blit(560, 135, fontOrange, "PREV", 4);
	app->fonts->Blit(650, 115, fontWhite, score.c_str(), 4);
	app->fonts->Blit(650, 135, fontOrange, highScore.c_str(), 4);
	app->fonts->Blit(650, 155, fontOrange, prevScore.c_str(), 4);
}

void Map::OffsetDrawPosition(iPoint& position, iPoint amount, bool condition, std::string const& str)
{
	if (!condition) return;

	if(str != "")
		//app->fonts->Blit(position + y, y + 20, fontOrange, "!");
	position += amount;
}
