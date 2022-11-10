#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	~Player() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	void OnCollision(PhysBody* physA, PhysBody* physB) final;

private:

	SDL_Texture *texture = nullptr;
	const char* texturePath = nullptr;

	PhysBody* pbody = nullptr;

	int pickCoinFxId = 0;

};

#endif // __PLAYER_H__