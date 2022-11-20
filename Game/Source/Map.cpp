#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>

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

	return true;
}

void Map::Draw()
{
	app->render->DrawTexture(backgroundImage, 0, 0);
}

// Called before quitting
bool Map::CleanUp()
{
	LOG("Unloading map");

	if(backgroundImage) app->tex->UnLoad(backgroundImage);

	return true;
}

// Load new map
bool Map::Load()
{
	uint aux = app->GetLevelNumber();

	auto levelFileName = texturePath + "level_" + std::to_string(aux) + "/bg"  + ".png";

	backgroundImage = app->tex->Load(levelFileName.c_str());

	musicPath += "level_" + std::to_string(aux) + ".ogg";

	return true;
}