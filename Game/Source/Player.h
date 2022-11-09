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
	
	virtual ~Player();

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	void OnCollision(PhysBody* physA, PhysBody* physB) final;


public:

private:
	SDL_Texture* texture;
	const char* texturePath;

	PhysBody* pbody;

	int pickCoinFxId;

};

#endif // __PLAYER_H__