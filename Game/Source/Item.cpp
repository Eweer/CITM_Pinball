#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{
	name.Create("item");
}

Item::~Item() = default;

bool Item::Awake() 
{
	//position.x = parameters.attribute("x").as_int();
	//position.y = parameters.attribute("y").as_int();
	//texturePath = parameters.attribute("texturepath").as_string();

	return true;
}

bool Item::Start() 
{
	//texture = app->tex->Load(texturePath);
	
	//pBody = app->physics->CreateCircle(position.x + 16, position.y + 16, 16, bodyType::DYNAMIC);

	//pBody->ctype = ColliderType::ITEM;

	return true;
}

bool Item::Update()
{
	//position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - 16;
	//position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - 16;

	//app->render->DrawTexture(texture, position.x, position.y);

	return true;
}

bool Item::CleanUp()
{
	return true;
}