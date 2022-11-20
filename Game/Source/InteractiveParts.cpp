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
#include <array>
#include <functional>

#include "PugiXml/src/pugixml.hpp"


InteractiveParts::InteractiveParts() : Entity(EntityType::UNKNOWN) {}

InteractiveParts::InteractiveParts(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

InteractiveParts::~InteractiveParts() = default;

bool InteractiveParts::Awake() 
{
	if(!parameters) return false;

	SetPaths();

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

	if(parameters.attribute("audio"))
	{
		std::string audioFile = fxPath + name + ".ogg";
		app->audio->LoadFx(audioFile.c_str());
	}

	CreateFlipperInfo();

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

	if(!flipperJoint) return true;
	
	if(app->physics->IsDebugActive())
	{
		auto anchorPos = app->physics->WorldVecToIPoint(flipperJoint->anchor->body->GetPosition());
		auto mainPos = app->physics->WorldVecToIPoint(pBody->body->GetPosition());

		app->render->DrawLine(mainPos.x, mainPos.y, anchorPos.x, anchorPos.y, 255, 0, 0);
	}
	
	if(app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
	{
		std::cout << "a" << std::endl;
		flipperJoint->joint->SetMotorSpeed(flipperJoint->motorSpeed * -1.0f);
	}

	return true;
}

bool InteractiveParts::CleanUp()
{
	switch(texture.type)
	{
		case RenderModes::IMAGE:
			app->tex->UnLoad(texture.image);
			break;

		case RenderModes::ANIMATION:
			texture.anim->CleanUp();
			break;

		default: 
			break;

	}

	return true;
}

void InteractiveParts::OnCollision(PhysBody *physA, PhysBody *physB)
{
	if(physB->ctype == ColliderType::BALL)
	{
		if(texture.type == RenderModes::ANIMATION && texture.anim) this->texture.anim->Start();
		if(ballCollisionAudio) app->audio->PlayFx(ballCollisionAudio);
	}
		
}

bool InteractiveParts::CreateColliders()
{
	//EntityType::ANIM are just animations of board, they don't have collisions.
	if(type == EntityType::ANIM) return true;

	auto collidersFileName = texLevelPath + "interactive_colliders_" + std::to_string(app->GetLevelNumber()) + ".xml";

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
			return CreateCollidersBasedOnShape(colliderNode);
		}
	}

	if(pBody) pBody->listener = this;

	return true;
}

bool InteractiveParts::CreateCollidersBasedOnShape(const pugi::xml_node &colliderNode)
{
	auto bodyTypeChar = colliderNode.attribute("bodytype").as_string();
	std::string colliderShape = colliderNode.attribute("shape").as_string();

	if(!bodyTypeChar || colliderShape.empty())
	{
		LOG("Can't read xml attributes on InteractiveParts::CreateCollidersBasedOnShape\n");
		LOG("ColliderShape is %s || BodyTypeChar is %s\n", bodyTypeChar ? "true" : "false", colliderShape.empty() ? "false" : "true");
		return false;
	}

	BodyType typeOfChildren = app->physics->GetEnumFromStr(bodyTypeChar);
	
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
		LOG("Attribute shape of %s not recognized in InteractiveParts::CreateCollidersBasedOnShape", colliderNode.name());
		return false;
	}
	return true;
}

PhysBody* InteractiveParts::CreateChainColliders(const std::string &xyStr, BodyType bodyT)
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

bool InteractiveParts::CreateFlipperInfo()
{
	
	if(name != "flipper" || this->flipperJoint) return false;

	LOG("Creating flipper info");

	FlipperInfo flipperHelper;

	flipperHelper.anchor = app->physics->CreateCircle(
		parameters.child("anchor").attribute("x").as_int(),
		parameters.child("anchor").attribute("y").as_int(),
		parameters.child("anchor").attribute("radius").as_int(),
		BodyType::STATIC
	);

	pugi::xml_node flipperNode = parameters.child("revolute_joint");
	std::vector<RevoluteJointSingleProperty> revoluteProperties;

	for(pugi::xml_attribute attr : flipperNode.attributes())
	{
		std::string attrName(attr.name());
		if(attrName == "motor_speed") flipperHelper.motorSpeed = attr.as_float();

		RevoluteJointSingleProperty propertyToAdd;

		propertyToAdd.type = app->physics->GetTypeFromProperty(attrName);

		switch(propertyToAdd.type)
		{
			case RevoluteJoinTypes::BOOL:
				propertyToAdd.b = attr.as_bool();
				break;

			case RevoluteJoinTypes::FLOAT:
				propertyToAdd.f = attr.as_float();
				break;

			case RevoluteJoinTypes::INT:
				propertyToAdd.i = attr.as_int();
				break;

			case RevoluteJoinTypes::IPOINT:
			case RevoluteJoinTypes::UNKNOWN:
				LOG("Something went wrong in InteractiveParts doing the revolute joint");
				break;
		}
		revoluteProperties.emplace_back(propertyToAdd);
	}
	if(std::string(this->parameters.name()) == "flipper_left")
		flipperHelper.joint = app->physics->CreateRevoluteJoint(flipperHelper.anchor, this->pBody, {0,0}, {8,13}, revoluteProperties);
	else
		flipperHelper.joint = app->physics->CreateRevoluteJoint(flipperHelper.anchor, this->pBody, {0,0}, {-8,13}, revoluteProperties);


	this->flipperJoint = std::make_unique<FlipperInfo>(flipperHelper);
	
	return true;
}

void InteractiveParts::AddTexturesAndAnimationFrames()
{
	if(!parameters.attribute("renderable").as_bool())
	{
		texture.type = RenderModes::NO_RENDER;
		return;
	}

	struct dirent **nameList;
	std::string texturePath = texLevelPath + name + "/";

	const char *dirPath = texturePath.c_str();
	int n = scandir(dirPath, &nameList, nullptr, DescAlphasort);
	static const std::regex r(R"(([A-Za-z]+(?:_[A-Za-z]*)*)_(?:(image|static|anim)([\d]*)).png)"); // www.regexr.com/72ogq
	std::string itemName(parameters.name());
	bool foundOne = false;

	while(n--)
	{
		if(nameList[n]->d_name[0] == '.')
		{
			free(nameList[n]);
			continue;
		}

		std::smatch m;
		std::string animFileName(nameList[n]->d_name);

		if(!std::regex_match(animFileName, m, r))
		{
			free(nameList[n]);
			continue;
		}

		std::string match1 = m[1]; //example: triangle_left

		if(match1 != itemName)
		{
			if(foundOne) return;	//As they are alphasorted, once we found one but the name isn't the same
			else continue;			//there won't be any more.
		}

		if(!foundOne) foundOne = true;

		std::string match2 = m[2]; // (image|static|anim)

		if(texture.type == RenderModes::UNKNOWN)
			texture.type = (match2 == "image") ? RenderModes::IMAGE : RenderModes::ANIMATION;

		std::string match0 = texturePath + std::string(m[0]); //example: /Assets/Maps/ + triangle_left_anim001.png

		switch(texture.type)
		{
			case RenderModes::ANIMATION:
				
				if(match2 == "anim") texture.anim->AddSingleFrame(match0.c_str());
				else texture.anim->AddStaticImage(match0.c_str());
				break;

			case RenderModes::IMAGE:
				texture.image = app->tex->Load(match0.c_str());
				break;

			default:
				break;
		}
		free(nameList[n]);
	}
	free(nameList);

	if(texture.type == RenderModes::ANIMATION)
	{
		texture.anim->SetSpeed(parameters.attribute("speed").as_float());
		auto animStyle = static_cast<AnimIteration>(parameters.attribute("animstyle").as_int());
		texture.anim->SetAnimStyle(animStyle);
		if(animStyle == AnimIteration::LOOP_FORWARD_BACKWARD || animStyle == AnimIteration::LOOP_FROM_START)
		{
			texture.anim->Start();
		}
	}
}
