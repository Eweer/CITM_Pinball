#include "App.h"
#include "InteractiveParts.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Physics.h"

#include "Log.h"
#include "Point.h"
#include "Animation.h"

#include <regex>
#include <string>
#include <vector>
#include <array>
#include <cmath>

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

	position = {
		parameters.attribute("x").as_int(),
		parameters.attribute("y").as_int()
	};

	if(!CreateColliders()) return false;

	AddTexturesAndAnimationFrames();

	if(parameters.attribute("hasfx"))
	{
		std::string audioFile = fxLevelPath + name + "." + parameters.attribute("hasfx").as_string();
		ballCollisionFx = app->audio->LoadFx(audioFile.c_str());
	}

	CreateFlipperInfo();

	if(!pBody) return true;

	if(parameters.attribute("function")) pBody->sensorFunction = (SensorFunction)parameters.attribute("function").as_int();
	else pBody->sensorFunction = SensorFunction::UNKNOWN;

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

	if(std::string(parameters.name()) == "anim_pinkpower" && bSpecialFunction == true)
	{
		texture.anim->DoLoopsOfAnimation(10, AnimIteration::LOOP_FROM_START);
		bSpecialFunction = false;
		return true;
	}

	if(flipperJoint)
	{
		if(app->physics->IsDebugActive() && flipperJoint)
		{
			auto anchorPos = app->physics->WorldVecToIPoint(flipperJoint->anchor->body->GetPosition());
			auto mainPos = app->physics->WorldVecToIPoint(pBody->body->GetPosition());
			app->render->DrawLine(mainPos.x, mainPos.y, anchorPos.x, anchorPos.y, 255, 0, 0);
		}

		if(app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
		{
			if(std::string(parameters.name()) == "flipper_left")
				flipperJoint->joint->SetMotorSpeed(flipperJoint->motorSpeed * -1.0f);
			else
				flipperJoint->joint->SetMotorSpeed(flipperJoint->motorSpeed);
		}
	}

	if(launcherJoint)
	{
		
		switch(app->input->GetKey(SDL_SCANCODE_DOWN))
		{
			case KeyState::KEY_DOWN:
			case KeyState::KEY_REPEAT:
				launcherJoint->joint->SetMotorSpeed(5);
				launcherJoint->joint->SetMaxMotorForce(10);
				break;

			case KeyState::KEY_UP:
				launcherJoint->joint->SetMotorSpeed(-1500);
				launcherJoint->joint->SetMaxMotorForce(2000);
				break;

			default:
				break;
		}
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - pBody->width - 15;
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - pBody->height;
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
		if(ballCollisionFx) app->audio->PlayFx(ballCollisionFx);

		switch(pBody->sensorFunction)
		{
			case SensorFunction::DEATH:
				break;

			case SensorFunction::POWER:
				bSpecialFunction = true;
				break;

			case SensorFunction::HP_UP:
				break;

			case SensorFunction::UNKNOWN:
				break;
		}
	}
		
}

bool InteractiveParts::CreateColliders()
{
	//EntityType::ANIM are just animations of board, they don't have collisions.
	if(type == EntityType::ANIM) return true;

	auto collidersFileName = texLevelPath + "colliders" + ".xml";

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

	const int posX = colliderNode.attribute("x").as_int();
	const int posY = colliderNode.attribute("y").as_int();
	
	if(colliderShape == "chain" || colliderShape == "polygon")
	{
		const std::string xyStr = colliderNode.attribute("xy").as_string();
		pBody = CreateChainColliders(xyStr, typeOfChildren, colliderShape);
	}
	else if(colliderShape == "circle")
	{
		const int radius = colliderNode.attribute("radius").as_int();
		pBody = app->physics->CreateCircle(posX, posY, radius, typeOfChildren);
	}
	else if(colliderShape == "rectangle_sensor")
	{
		const int width = colliderNode.attribute("w").as_int();
		const int height = colliderNode.attribute("h").as_int();
		pBody = app->physics->CreateRectangleSensor(posX, posY, width, height, typeOfChildren);
	}
	else if(colliderShape == "rectangle")
	{
		const int width = colliderNode.attribute("w").as_int();
		const int height = colliderNode.attribute("h").as_int();
		pBody = app->physics->CreateRectangle(posX, posY, width, height, typeOfChildren, 0.0f, 0.1f, (int)Layers::BOARD, (int)Layers::BALL);
	}
	else
	{
		LOG("Attribute shape of %s not recognized in InteractiveParts::CreateCollidersBasedOnShape", std::string(colliderNode.name()));
		return false;
	}

	pBody->listener = this;

	switch(this->type)
	{
		case EntityType::TRIANGLE:
		case EntityType::CIRCLE:
		case EntityType::RAMP:
		case EntityType::DIVIDER:
			pBody->ctype = ColliderType::ITEM;
			break;

		case EntityType::SENSOR:
			pBody->ctype = ColliderType::SENSOR;
			break;

		case EntityType::UNKNOWN:
			pBody->ctype = ColliderType::UNKNOWN;
			break;

		default:
			pBody->ctype = ColliderType::BOARD;
	}
	return true;
}

PhysBody *InteractiveParts::CreateChainColliders(const std::string &xyStr, BodyType bodyT, std::string const &shape)
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

	PhysBody *border;
	if(shape == "chain")
	{
		if(name == "bridge") border = app->physics->CreateChain(0, 0, points.data(), std::distance(xyStrBegin, xyStrEnd), bodyT, 0.0f, (uint)Layers::TOP, (uint)Layers::BALL);
		else border = app->physics->CreateChain(0, 0, points.data(), std::distance(xyStrBegin, xyStrEnd), bodyT);
	}
	else
	{
		int posX = parameters.child("anchor").attribute("x").as_int();
		int posY = parameters.child("anchor").attribute("y").as_int();

		if(name == "flipper") border = app->physics->CreatePolygon(posX, posY, points.data(), std::distance(xyStrBegin, xyStrEnd), bodyT, 5.0f, (uint)Layers::KICKERS, (uint)Layers::BALL);
		else border = app->physics->CreatePolygon(posX, posY, points.data(), std::distance(xyStrBegin, xyStrEnd), bodyT);
	}
	
	return border;
}

bool InteractiveParts::CreateFlipperInfo()
{
	
	if((name != "flipper" || this->flipperJoint) && (name != "launcher" || this->launcherJoint))
		return false;

	LOG("Creating flipper info");

	FlipperInfo flipperHelper;
	LauncherInfo launcherHelper;

	if(name == "flipper")
	{
		flipperHelper.anchor = app->physics->CreateCircle(
			parameters.child("anchor").attribute("x").as_int(),
			parameters.child("anchor").attribute("y").as_int(),
			parameters.child("anchor").attribute("radius").as_int(),
			BodyType::STATIC
		);
	}
	else
	{
		launcherHelper.anchor = app->physics->CreateRectangle(
			parameters.child("anchor").attribute("x").as_int(),
			parameters.child("anchor").attribute("y").as_int(),
			parameters.child("anchor").attribute("w").as_int(),
			parameters.child("anchor").attribute("h").as_int(),
			BodyType::STATIC
		);
	}

	pugi::xml_node flipperNode = parameters.child("joint");
	std::vector<RevoluteJointSingleProperty> revoluteProperties;

	for(pugi::xml_attribute attr : flipperNode.attributes())
	{
		std::string attrName(attr.name());
		if(attrName == "motor_speed")
		{
			if(name == "flipper") flipperHelper.motorSpeed = attr.as_float();
			else launcherHelper.motorSpeed = attr.as_float();
		}
				

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
		flipperHelper.joint = app->physics->CreateRevoluteJoint(flipperHelper.anchor, this->pBody, {0,0}, {50,13}, revoluteProperties);
	else if(std::string(this->parameters.name()) == "flipper_right")
		flipperHelper.joint = app->physics->CreateRevoluteJoint(flipperHelper.anchor, this->pBody, {0,0}, {8,13}, revoluteProperties);
	else
	{
		iPoint offset;
		offset.x = pBody->width + 15;
		offset.y = pBody->height;
		launcherHelper.joint = app->physics->CreatePrismaticJoint(launcherHelper.anchor, this->pBody, offset, {0,0}, revoluteProperties);

	}
	if(this->name == "flipper") this->flipperJoint = std::make_unique<FlipperInfo>(flipperHelper);
	else this->launcherJoint = std::make_unique<LauncherInfo>(launcherHelper);
	
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
	std::string interactiveFolder = texLevelPath + name + "/";

	const char *dirPath = interactiveFolder.c_str();
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
		{
			texture.type = (match2 == "image") ? RenderModes::IMAGE : RenderModes::ANIMATION;

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

		std::string match0 = interactiveFolder + std::string(m[0]); //example: /Assets/Maps/ + triangle_left_anim001.png

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
}
