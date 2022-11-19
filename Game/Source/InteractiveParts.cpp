#include "InteractiveParts.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"

#include "Log.h"
#include "Point.h"
#include "Animation.h"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "PugiXml/src/pugixml.hpp"


InteractiveParts::InteractiveParts() : Entity(EntityType::UNKNOWN) {}

InteractiveParts::InteractiveParts(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

InteractiveParts::~InteractiveParts() = default;

bool InteractiveParts::Awake() 
{
	if(!parameters) return false;

	interactiveCollidersFolder = parameters.parent().attribute("assetpath").as_string();

	return true;
}

bool InteractiveParts::Start() 
{
	
	if(!CreateColliders()) return false;
	
	position = { 
		parameters.attribute("x").as_int(), 
		parameters.attribute("y").as_int() 
	};

	AddTexturesAndAnimationFrames();

	//pBody->ctype = ColliderType::INTERACTIVE_PARTS;

	return true;
}

bool InteractiveParts::Update()
{	
	switch(texture.type)
	{
		case RenderModes::IMAGE:
			app->render->DrawTexture(texture.image, position.x, position.y);
			break;
			
		case RenderModes::ANIMATION:
			app->render->DrawTexture(texture.anim->GetCurrentFrame(), position.x, position.y);
			break;

		default:
			break;
	}

	return true;
}

bool InteractiveParts::CleanUp()
{
	return true;
}

bool InteractiveParts::CreateColliders()
{
	auto collidersFileName = interactiveCollidersFolder + "interactive_colliders_" + std::to_string(app->GetLevelNumber()) + ".xml";

	pugi::xml_parse_result parseResult = collidersFile.load_file(collidersFileName.c_str());

	if(!parseResult)
	{
		LOG("Error in InteractiveParts::Start(): %s", parseResult.description());
		return false;
	}

	pugi::xml_node infoColliders = collidersFile.child("collider_info");

	for(auto const &colliderNode : infoColliders.children())
	{
		if(std::string(colliderNode.name()) == std::string(parameters.name()))
		{
			auto bodyTypeChar = colliderNode.attribute("bodytype").as_string();
			bodyType typeOfChildren = app->physics->GetEnumFromStr(bodyTypeChar);

			CreateCollidersBasedOnName(colliderNode, typeOfChildren);
			break;
		}
	}
}

void InteractiveParts::CreateCollidersBasedOnName(const pugi::xml_node &colliderNode, bodyType typeOfChildren)
{
	std::string colliderShape(colliderNode.attribute("shape").as_string());
	if(colliderShape == "chain")
	{
		const std::string xyStr = colliderNode.attribute("xy").as_string();
		pBody = CreateChainColliders(xyStr, typeOfChildren);
	}
	else if(colliderShape == "circle")
	{
		const int posX = colliderNode.attribute("x").as_int();
		const int posY = colliderNode.attribute("y").as_int();
		const int radius = colliderNode.attribute("radius").as_int();
		pBody = app->physics->CreateCircle(posX, posY, radius, typeOfChildren);
	}
	else
	{
		LOG("Attribute shape of %s not recognized in Map::Load()", colliderNode.name());
	}
}

PhysBody* InteractiveParts::CreateChainColliders(const std::string &xyStr, bodyType bodyT)
{
	static const std::regex r("\\d{1,3}");
	auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
	auto xyStrEnd = std::sregex_iterator();

	std::vector<int> points;

	for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
	{
		std::smatch match = *i;
		points.push_back(stoi(match.str()));
	}

	PhysBody *border = app->physics->CreateChain(0, 0, points.data(), std::distance(xyStrBegin, xyStrEnd), bodyT);

	return border;
}

void InteractiveParts::AddTexturesAndAnimationFrames()
{
	if(!parameters.attribute("renderable").as_bool())
	{
		texture.type = RenderModes::NO_RENDER;
		return;
	}

	struct dirent **nameList;
	std::string texturePath = interactiveCollidersFolder + name + "/";

	const char *dirPath = texturePath.c_str();
	int n = scandir(dirPath, &nameList, nullptr, DescAlphasort) - 2;
	static const std::regex r(R"(([A-Za-z]+(?:_[A-Za-z]*)*)_(?:(image|anim)([\d]*)).png)"); // www.regexr.com/72ogq
	std::string itemName(parameters.name());
	bool foundOne = false;

	while(n--)
	{
		std::smatch m;
		std::string animFileName(nameList[n]->d_name);

		if(!std::regex_match(animFileName, m, r))
		{
			free(nameList[n]);
			continue;
		}
		std::string match = m[1];

		if(match != itemName)
		{
			if(foundOne) return;	//As they are alphasorted, once we found one but the name isn't the same
			else continue;			//there won't be any more.
		}

		if(!foundOne) foundOne = true;

		if(texture.type == RenderModes::UNKNOWN)
		{
			match = m[2];
			texture.type = (match == "image") ? RenderModes::IMAGE : RenderModes::ANIMATION;
		}

		match = texturePath;
		match += m[0];

		switch(texture.type)
		{
			case RenderModes::ANIMATION:
				texture.anim->AddSingleFrame(match.c_str());
				break;

			case RenderModes::IMAGE:
				texture.image = app->tex->Load(match.c_str());
				break;

			default:
				break;
		}
		free(nameList[n]);
	}
	free(nameList);
}
