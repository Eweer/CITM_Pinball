#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"
#include "List.h"

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called after Awake
	bool Start() final;

	// Called every frame
	bool Update(float dt) final;

	// Called before quitting
	bool CleanUp() final;

	// Additional methods
	Entity *CreateEntity(pugi::xml_node const &itemNode);

	void DestroyEntity(Entity* entity);

	void AddEntity(Entity* entity);

	uint GetScore() const;

	std::pair<uint, uint> GetScoreList() const;

	List<Entity*> entities;
	std::pair<Entity*, Entity*> flippers;
	Entity *launcher = nullptr;
	Entity *ball = nullptr;

	std::vector<Entity *> dividers;
	Entity *rotatePower = nullptr;
	Entity *pinkPower = nullptr;
};

#endif // __ENTITYMANAGER_H__
