#include "EntityManager.h"
#include "Ball.h"
#include "Item.h"
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

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr; 

	switch (type)
	{
	case EntityType::BALL:
		entity = new Ball();
		break;

	case EntityType::ITEM:
		entity = new Item();
		break;

	default: break;
	}

	// Created entities are added to the list
	AddEntity(entity);

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