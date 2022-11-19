#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Point.h"
#include "SString.h"
#include "Input.h"
#include "Render.h"
#include "Log.h"
#include "Animation.h"

#include <regex>
#include <string>
#include <unordered_map>
#include <iostream>

#include <memory>

class PhysBody;

enum class RenderModes
{
	IMAGE = 0,
	ANIMATION,
	NO_RENDER,
	UNKNOWN
};


//DO NOT TOUCH AT ALL. EVERYTHING WILL BREAK. IT MANAGED TO WORK. LET'S IGNORE THIS
struct Texture
{
	RenderModes type;
	union
	{
		std::unique_ptr<Animation> anim = std::make_unique<Animation>();
		SDL_Texture *image;
	};

	Texture::Texture() = default;
	Texture::Texture(const Texture &t) : type(t.type)
	{
		switch(type)
		{
			case RenderModes::ANIMATION:
				break;
			case RenderModes::IMAGE:
				image = t.image;
				break;
		}

	}
	Texture::~Texture(){}
};
// YOU CAN START TYPING AGAIN AFTER THIS LINE

enum class EntityType
{
	BALL = 0,
	CIRCLE_COMBO,
	TREES,
	PLUNGER,
	FLIPPER,
	RAMP,
	TRIANGLE,
	BORDER,
	UNKNOWN
};

class Entity
{
public:

	explicit Entity() = default;

	explicit Entity(EntityType type) : type(type) {}

	explicit Entity(pugi::xml_node const &itemNode) : parameters(itemNode)
	{
		const std::unordered_map<std::string, EntityType> entityTypeStrToEnum{
			{"ball", EntityType::BALL},
			{"circle", EntityType::CIRCLE_COMBO},
			{"trees", EntityType::TREES},
			{"plunger", EntityType::PLUNGER},
			{"flipper", EntityType::FLIPPER},
			{"ramp", EntityType::RAMP},
			{"triangle", EntityType::TRIANGLE},
			{"border", EntityType::BORDER},
			{"unknown", EntityType::UNKNOWN}
		};

		std::smatch m;
		std::string itemName(itemNode.name());
		if(!std::regex_search(itemName, m, std::regex(R"([A-Za-z]+)")))
		{
			LOG("XML %s name is not correct. [A-Za-z]+", itemNode.name());
			return;
		}

		if(!entityTypeStrToEnum.count(m[0]))
		{
			LOG("%s string does not have a mapped enum", m[0]);
			return;
		}

		if(static_cast<uint>(entityTypeStrToEnum.at(m[0])) >= static_cast<uint>(EntityType::UNKNOWN)) 
		{
			LOG("%s does not have a valid EntityType", m[0]);
			return;
		}
		this->name = m[0];
		this->type = entityTypeStrToEnum.at(name);

		texture.type = RenderModes::UNKNOWN;
		texture.anim = std::make_unique<Animation>();
	}

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

	pugi::xml_node parameters;
	bool active = true;

	std::string name = "unknown";
	EntityType type = EntityType::UNKNOWN;
	iPoint position;

	std::string texturePath;

	Texture texture;

	PhysBody *pBody = nullptr;
};



#endif // __ENTITY_H__