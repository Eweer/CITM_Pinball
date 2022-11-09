#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	bool isPicked = false;

private:

	PhysBody *pBody = nullptr;

	SDL_Texture *texture = nullptr;
	const char* texturePath = nullptr;
};

#endif // __ITEM_H__