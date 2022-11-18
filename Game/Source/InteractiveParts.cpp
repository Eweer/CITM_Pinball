#include "InteractiveParts.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"

InteractiveParts::InteractiveParts() : Entity(EntityType::INTERACTIVE_PARTS)
{
	name.Create("item");
}

InteractiveParts::~InteractiveParts() = default;

bool InteractiveParts::Awake() 
{
	//position.x = parameters.attribute("x").as_int();
	//position.y = parameters.attribute("y").as_int();
	//texturePath = parameters.attribute("texturepath").as_string();

	return true;
}

bool InteractiveParts::Start() 
{
	//texture = app->tex->Load(texturePath);
	
	//pBody = app->physics->CreateCircle(position.x + 16, position.y + 16, 16, bodyType::DYNAMIC);

	//pBody->ctype = ColliderType::INTERACTIVE_PARTS;

	return true;
}

bool InteractiveParts::Update()
{
	//position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - 16;
	//position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - 16;

	//app->render->DrawTexture(texture, position.x, position.y);

	return true;
}

bool InteractiveParts::CleanUp()
{
	return true;
}