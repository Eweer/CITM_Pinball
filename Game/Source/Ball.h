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

	void AddMultiplier(uint n) final;

	int GetTimeUntilReset() const;

	std::pair<uint, uint> GetScoreList() const final;

private:

	void CreatePhysBody();
	void SetStartingPosition();

	float score = 0;
	uint scoreMultiplier = 1;
	std::pair<uint, uint> scoreList;

	uint hp = 3;

	SDL_Texture *hpTexture = nullptr;

	int timeUntilReset = -1;
};

#endif // __BALL_H__