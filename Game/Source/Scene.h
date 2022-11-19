#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Ball.h"
#include "InteractiveParts.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& config) final;

	// Called before the first frame
	bool Start() final;

	// Called before all Updates
	bool PreUpdate() final;

	// Called each loop iteration
	bool Update(float dt) final;

	// Called before all Updates
	bool PostUpdate() final;

	// Called before quitting
	bool CleanUp() final;
};

#endif // __SCENE_H__