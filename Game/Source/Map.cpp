
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
		pugi::xml_node infoColliders = collidersFile.child("collider_info");

		for(pugi::xml_node colliderBodyType : infoColliders.children())
		{
			const char *bodyTypeChar = colliderBodyType.name();
			bodyType typeOfChildren = app->physics->GetEnumFromStr(std::string(bodyTypeChar));
			for(pugi::xml_node colliderAttributes : colliderBodyType.children())
			{
				CreateCollidersBasedOnName(colliderAttributes, typeOfChildren);
			}
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

void Map::CreateCollidersBasedOnName(const pugi::xml_node &colliderAttributes, bodyType typeOfChildren)
{
	std::string colliderName(colliderAttributes.name());
	if(colliderName == "chain")
	{
		const std::string xyStr = colliderAttributes.attribute("xy").as_string();
		CreateChainColliders(xyStr, typeOfChildren);
	}
	else if(colliderName == "circle")
	{
		//Attributes: name | x | y | radius
		const int posX = colliderAttributes.attribute("x").as_int();
		const int posY = colliderAttributes.attribute("y").as_int();
		const int radius = colliderAttributes.attribute("radius").as_int();
		app->physics->CreateCircle(posX, posY, radius, typeOfChildren);
	}
	else
	{
		LOG("Attribute name %s not recognized in Map::Load()", colliderAttributes.name());
	}
}

void Map::CreateChainColliders(const std::string &xyStr, bodyType bodyT)
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

	PhysBody *border = app->physics->CreateChain(0, 0, points, pointsEnd, bodyT);
	border->ctype = ColliderType::PLATFORM;

	delete[] points;
}
