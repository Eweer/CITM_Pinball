#ifndef __BALL_H__
#define __BALL_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"
#include <array>

struct SDL_Texture;

class Ball : public Entity
{
public:

	explicit Ball();

	explicit Ball(const pugi::xml_node &itemNode);
	
	~Ball() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	void OnCollision(PhysBody* physA, PhysBody* physB) final;

	void ResetScore();

	uint GetScore() const final;

private:
	uint score = 0;
	std::array<float, 3> scoreList;
};

#endif // __BALL_H__