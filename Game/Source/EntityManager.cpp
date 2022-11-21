#include "EntityManager.h"
#include "Ball.h"
#include "InteractiveParts.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"

#include "Defs.h"
#include "Log.h"

EntityManager::EntityManager() : Module()
{
	name.Create("entitymanager");
}

// Destructor
EntityManager::~EntityManager() = default;

// Called before render is available
bool EntityManager::Awake(pugi::xml_node& config)
{
	LOG("Loading Entity Manager");

	//Iterates over the entities and calls the Awake
	for (ListItem<Entity*>* item = entities.start; item; item = item->next)
	{
		const Entity* pEntity = item->data;
		if(!pEntity->active) continue;
		if(!item->data->Awake()) return false;
	}

	return true;

}

bool EntityManager::Start() 
{
	//Iterates over the entities and calls Start
	for(ListItem<Entity *>*item = entities.start; item; item = item->next)
	{
		const Entity *pEntity = item->data;
		if(!pEntity->active) continue;
		if(!item->data->Start()) return false;
	}

	return true;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	ListItem<Entity*>* item = entities.end;

	while (item)
	{
		if(!item->data->CleanUp()) return false;
		item = item->prev;
	}

	entities.Clear();

	return true;
}

Entity *EntityManager::CreateEntity(pugi::xml_node const &itemNode = pugi::xml_node())
{
	Entity *entity = nullptr;

	if(std::string(itemNode.name()) == "ball") entity = new Ball(itemNode);
	else entity = new InteractiveParts(itemNode);

	// Created entities are added to the list
	AddEntity(entity);

	std::string itemName(itemNode.name());

	if(!ball && itemName == "ball") ball = entity;

	if(!launcher && itemName == "launcher_top") launcher = entity;

	if(!flippers.first || !flippers.second)
	{
		std::string nameStart = itemName.substr(0, std::string("flipper").size());
		if(nameStart == "flipper")
		{
			std::string nameEnd = itemName.substr(std::string("flipper").size() + 1);
			if(nameEnd == "left") flippers.first = entity;
			else flippers.second = entity;
		}
	}
	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	for(ListItem<Entity*>* item = entities.start; item; item = item->next)
	{
		if(item->data == entity)
		{
			entities.Del(item);
			return;
		}
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if(entity != nullptr) entities.Add(entity);
}

bool EntityManager::Update(float dt)
{
	//Iterates over the entities and calls Update
	for(ListItem<Entity *> *item = entities.start; item; item = item->next)
	{
		const Entity *pEntity = item->data;
		if(!pEntity->active) continue;
		if(!item->data->Update()) return false;
	}

	return true;
}

uint EntityManager::GetScore() const
{
	return ball->GetScore();
}

std::pair<uint, uint> EntityManager::GetScoreList() const
{
	return ball->GetScoreList();
}
