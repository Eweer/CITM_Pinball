#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Point.h"
#include "SString.h"
#include "Input.h"
#include "Render.h"

class PhysBody;

enum class EntityType
{
	PLAYER,
	ITEM,
	UNKNOWN
};

class Entity
{
public:

	explicit Entity(EntityType type) : type(type) {}

	virtual ~Entity() = default;

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	virtual bool LoadState(pugi::xml_node&)
	{
		return true;
	}

	virtual bool SaveState(pugi::xml_node&)
	{
		return true;
	}

	void Entity::Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Entity::Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB) 
	{
		//To override
	};

	SString name;
	EntityType type;
	bool active = true;
	pugi::xml_node parameters;

	iPoint position;       
	bool renderable = true;

	SDL_Texture *texture = nullptr;
	const char *texturePath = nullptr;

	PhysBody *pBody = nullptr;
};

#endif // __ENTITY_H__