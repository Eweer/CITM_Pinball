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

constexpr uint BALL_SIZE = 30;

Ball::Ball() : Entity(EntityType::UNKNOWN) {}

Ball::Ball(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

Ball::~Ball() = default;

bool Ball::Awake() 
{
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	scoreList.first = parameters.attribute("highscore").as_uint();
	scoreList.second = 0;
	SetPaths();

	return true;
}

bool Ball::Start() {

	//initilize textures
	texture.type = RenderModes::IMAGE;

	std::string ballImage = texLevelPath + name + ".png";

	texture.image = app->tex->Load(ballImage.c_str());

	CreatePhysBody();

	return true;
}

bool Ball::Update()
{
	if(timeUntilReset > 120)
	{
		SetStartingPosition();
		timeUntilReset = -1;
		if(hp <= 0)
		{
			if((uint)score > scoreList.first)
			{
				scoreList.first = (uint)score;
				app->SaveToConfig("scene", "ball", "highscore", std::to_string(scoreList.first));
			}
			scoreList.second = (uint)score;
			ResetScore();
			hp = 3;
		}
	}
	else if(timeUntilReset >= 0)
	{
		timeUntilReset++;
	}
	else
	{
		score += 0.002f * (float)scoreMultiplier;
	}

	//Update ball position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - BALL_SIZE/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - BALL_SIZE/2;

	app->render->DrawTexture(texture.image, position.x , position.y);

	for(int i = 0; i < hp; i++)
	{
		app->render->DrawTexture(texture.image, 710, 930 - i*(BALL_SIZE + 10));
	}

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
}

void Ball::OnCollision(PhysBody* physA, PhysBody* physB) {
	if(timeUntilReset >= 0) return;
	switch (physB->ctype)
	{
		case ColliderType::ITEM:
			if(score < 99999)
			{
				score += (float)(100 * scoreMultiplier);
				if(score > 99999) score = 99999;
			}
			LOG("Collision ITEM");
			break;
		case ColliderType::ANIM:
			LOG("Collision ANIM");
			break;
		case ColliderType::SENSOR:
			LOG("Collision SENSOR");
			switch(physB->sensorFunction)
			{
				case SensorFunction::DEATH:
					timeUntilReset = 0;
					hp--;
					break;

				case SensorFunction::POWER:
					break;

				case SensorFunction::HP_UP:
					break;

				case SensorFunction::UNKNOWN:
					break;

				default:
					break;
			}
			break; 
		case ColliderType::BOARD:
				LOG("Collision BOARD");
				break;
		case ColliderType::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
		default:
			LOG("HOW DID YOU GET HERE?!?!?!?");
	}
}

void Ball::ResetScore()
{
	score = 0;
}

uint Ball::GetScore() const
{
	return score;
}

void Ball::AddMultiplier(uint n)
{
	scoreMultiplier += n;
}

int Ball::GetTimeUntilReset() const
{
	return timeUntilReset;
}

std::pair<uint, uint> Ball::GetScoreList() const
{
	return scoreList;
}

void Ball::CreatePhysBody()
{
	//initialize physics body
	pBody = app->physics->CreateCircle(position.x+BALL_SIZE/2, position.y+BALL_SIZE/2, BALL_SIZE/2, BodyType::DYNAMIC, 0.7f, (uint16)Layers::BALL, (uint16)Layers::BOARD | (uint16)Layers::SENSOR);

	//This makes the Physics module to call the OnCollision method
	pBody->listener = this;

	//Assign collider type
	pBody->ctype = ColliderType::BALL;
}

void Ball::SetStartingPosition()
{
	if(pBody->body) app->physics->DestroyBody(pBody->body);
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	CreatePhysBody();
}
