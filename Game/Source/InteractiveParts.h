#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"

#include "Point.h"

#include <regex>
#include <string>
#include "Physics.h"

#include "SDL/include/SDL.h"
#include "PugiXml/src/pugixml.hpp"

struct SDL_Texture;

struct FlipperInfo
{
	PhysBody *anchor;
	b2RevoluteJoint *joint;
	float32 motorSpeed;
};

struct LauncherInfo
{
	PhysBody *anchor;
	b2PrismaticJoint *joint;
	float32 motorSpeed;
};

class InteractiveParts : public Entity
{
public:

	explicit InteractiveParts();

	explicit InteractiveParts(const pugi::xml_node &itemNode);

	~InteractiveParts() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	void OnCollision(PhysBody *physA, PhysBody *physB) final;

private:

	bool CreateColliders();
	bool CreateCollidersBasedOnShape(const pugi::xml_node &colliderAttributes);
	PhysBody *CreateChainColliders(const std::string &xyStr, BodyType bodyT, std::string const &shape);

	bool CreateFlipperInfo();

	void AddTexturesAndAnimationFrames();

	int scoreValue = 0;
	uint ballCollisionFx = 0;


	std::unique_ptr<FlipperInfo> flipperJoint;
	std::unique_ptr<LauncherInfo> launcherJoint;

	pugi::xml_document collidersFile;
};

#endif // __ITEM_H__