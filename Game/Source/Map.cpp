
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"

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
	bool ret = true;

	mapFolder = config.child("mapfolder").attribute("path").as_string();

	return ret;
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
	auto levelFileName = mapFolder + "bg_" + std::to_string(levelNumber) + ".png";

	background = app->tex->Load(levelFileName.c_str());

	auto collidersFileName = mapFolder + "map_" + std::to_string(levelNumber) + ".xml";

	pugi::xml_parse_result parseResult = collidersFile.load_file(collidersFileName.c_str());

	if(!parseResult)
	{
		LOG("Error in Map::Load(): %s", parseResult.description());
		return false;
	}

	try
	{
		pugi::xml_node staticColliders = collidersFile.child("collider_info").child("static");
		for(pugi::xml_node collidersNode = staticColliders.child("chain"); collidersNode; collidersNode = collidersNode.next_sibling("chain"))
		{
			const std::string xyStr = collidersNode.attribute("xy").as_string();
			CreateChainColliders(xyStr);
		}

	} catch(const std::regex_error &rerr)
	{
		if(rerr.code())
		{
			LOG("Error in Map::Load() regex");
		}
	} catch(...)
	{
		LOG("Error in Map::Load() XML");
	}

	return true;
}

void Map::CreateChainColliders(const std::string &xyStr)
{
	static const std::regex r("\\d{1,3}");
	auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
	auto xyStrEnd = std::sregex_iterator();

	auto *points = new int[std::distance(xyStrBegin, xyStrEnd)];
	int pointsEnd = 0;

	for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
	{
		std::smatch match = *i;
		points[pointsEnd] = stoi(match.str());
		pointsEnd++;
	}

	PhysBody *border = app->physics->CreateChain(0, 0, points, pointsEnd, bodyType::STATIC);
	border->ctype = ColliderType::PLATFORM;

	delete[] points;
}
