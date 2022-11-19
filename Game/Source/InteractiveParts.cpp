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
	
	position = { 
		parameters.attribute("x").as_int(), 
		parameters.attribute("y").as_int() 
	};

	texturePath = interactiveCollidersFolder + name + "/" + parameters.name() + "_anim001.png";

	renderable = parameters.attribute("renderable").as_bool();

	if(renderable)
	{
		texture = app->tex->Load(texturePath.c_str());
		if(!texture) LOG("%s texture didn't load properly.", parameters.name());
	}

	//pBody->ctype = ColliderType::INTERACTIVE_PARTS;

	return true;
}

bool InteractiveParts::Update()
{
	if(renderable) app->render->DrawTexture(texture, position.x, position.y);

	return true;
}

bool InteractiveParts::CleanUp()
{
	return true;
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
		//Attributes: name | x | y | radius
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