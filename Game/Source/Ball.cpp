#include "Ball.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"

constexpr uint BALL_SIZE = 40;

Ball::Ball() : Entity(EntityType::UNKNOWN) {}

Ball::Ball(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

Ball::~Ball() = default;

bool Ball::Awake() 
{
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();

	return true;
}

bool Ball::Start() {

	//initilize textures
	texture.type = RenderModes::IMAGE;
	texture.image = app->tex->Load(texturePath.c_str());

	//initialize physics body
	pBody = app->physics->CreateCircle(position.x+BALL_SIZE/2, position.y+BALL_SIZE/2, BALL_SIZE/2, BodyType::DYNAMIC);

	//This makes the Physics module to call the OnCollision method
	pBody->listener = this; 

	//Assign collider type
	pBody->ctype = ColliderType::BALL;

	//initialize audio effect - !! Path is hardcoded, should be loaded from config.xml
	//pickCoinFxId = app->audio->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

	return true;
}

bool Ball::Update()
{
	//Update ball position in pixels
	
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - BALL_SIZE/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - BALL_SIZE/2;

	app->render->DrawTexture(texture.image, position.x , position.y);
	
	return true;
}

bool Ball::CleanUp()
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

	return true;
}

void Ball::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
		case ColliderType::ITEM:
			LOG("Collision ITEM");
			break;
		case ColliderType::PLATFORM:
			LOG("Collision PLATFORM");
			break;
		case ColliderType::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
		default:
			LOG("HOW DID YOU GET HERE?!?!?!?");
	}
	


}
