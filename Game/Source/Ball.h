#ifndef __BALL_H__
#define __BALL_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Ball : public Entity
{
public:

	Ball();
	
	~Ball() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	void OnCollision(PhysBody* physA, PhysBody* physB) final;

};

#endif // __BALL_H__