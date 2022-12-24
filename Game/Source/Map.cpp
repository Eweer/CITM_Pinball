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
#include <functional>

#include "SDL_image/include/SDL_image.h"

#include "PugiXml/src/pugixml.hpp"

#include <iostream>


/*		Move position by amount if condition is true.
*		Position: Current Position
*		Amount: How much to move the position
*		lambdaValue: Number that will be evaluated
*		Condition: Lambda function that will offset if condition is true
*		str: Text to write before offsetting if condition true
*		font: font to use
*/
template<class UnaryPred> 
void OffsetDrawPosition(iPoint &position, iPoint amount, float lambdaValue = 1, UnaryPred predicate = true, std::string const &str = "", uint font = 0)
{
	if(!predicate(lambdaValue)) return;

	if(str != "") app->fonts->Blit(position.x, position.y, font, str.c_str());

	position += amount;
}


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
	DrawScores(565, 125, -4, -10.0f);
	DrawFPS(5, 5);
	DrawGravity(5, 75);
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

	if (gravity.x < 0) app->fonts->Blit(pos.x, pos.y, fontOrange, "!");

	app->fonts->Blit(pos.x, pos.y, fontOrange, gravX.c_str());


	OffsetDrawPosition(pos, iPoint(15, 0), gravity.x, [](float n) { return n < 0; });
	OffsetDrawPosition(pos, iPoint(15, 0), gravity.x, [](float n) { return abs(n) > 9; });

	pos.x += 45;

	app->fonts->Blit(pos.x, pos.y, fontWhite, ".");

	pos.x += 10;

	if (gravity.y < 0) app->fonts->Blit(pos.x, pos.y, fontOrange, "!");

	app->fonts->Blit(pos.x, pos.y, fontOrange, gravY.c_str());

	OffsetDrawPosition(pos, iPoint(15, 0), gravity.y, [](float n) { return n < 0; });
	OffsetDrawPosition(pos, iPoint(15, 0), gravity.y, [](float n) { return abs(n) > 9; });

	pos.x += 45;

	app->fonts->Blit(pos.x, pos.y, fontWhite, "'"); 
}

void Map::DrawFPS(int x, int y) const
{
	std::string vSyncActive;

	uint vSyncFont = fontWhite;

	if(app->render->RestartForVSync()) 
	{
		vSyncActive = "Restart for vsync";
		vSyncFont = fontOrange;
	}
	else if(app->render->IsVSyncActive()) vSyncActive = "vsync enabled";
	else vSyncActive = "vsync disabled";

	app->fonts->Blit(x, y, vSyncFont, vSyncActive.c_str());

	std::string currentFPS = std::to_string(app->render->GetCurrentFPS());
	app->fonts->Blit(x, y + 20, fontWhite, "CURRENT FPS ");
	app->fonts->Blit(x + 180, y + 20, fontWhite, currentFPS.c_str());

	std::string targetFPS = std::to_string(app->render->GetTargetFPS());
	app->fonts->Blit(x, y + 40, fontOrange, "TARGET  FPS ");
	app->fonts->Blit(x + 180, y + 40, fontOrange, targetFPS.c_str());
}

void Map::DrawScores(int x, int y, int offsetY, double angle) const
{
	std::string score = std::to_string(app->entityManager->GetScore());
	app->fonts->Blit(x, y, fontWhite, "SC0RE ", offsetY, angle);
	app->fonts->Blit(x + 90, y - 20, fontWhite, score.c_str(), offsetY, angle);

	std::pair<uint, uint> scoreList = app->entityManager->GetScoreList();

	std::string highScore = std::to_string(scoreList.first);
	app->fonts->Blit(x, y + 20, fontOrange, "HIGH", offsetY, angle);
	app->fonts->Blit(x + 90, y, fontOrange, highScore.c_str(), offsetY, angle);

	std::string lastScore = std::to_string(scoreList.second);
	app->fonts->Blit(x, y + 40, fontOrange, "LAST", offsetY, angle);
	app->fonts->Blit(x + 90, y +20, fontOrange, lastScore.c_str(), offsetY, angle);
}
