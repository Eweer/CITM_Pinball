#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>
#include <regex>

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

	mapFolder = config.child("mapfolder").attribute("path").as_string();

	return true;
}

void Map::Draw()
{
	app->render->DrawTexture(background, 0, 0);
}

// Called before quitting
bool Map::CleanUp()
{
	LOG("Unloading map");

	return true;
}

// Load new map
bool Map::Load()
{
	uint aux = app->GetLevelNumber();
	std::string str(std::to_string(aux));
	auto levelFileName = mapFolder + "bg_" + str + ".png";

	background = app->tex->Load(levelFileName.c_str());

	return true;
}