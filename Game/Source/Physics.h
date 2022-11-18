#pragma once
#include "Module.h"
#include "Entity.h"

#include <unordered_map>

#include "Box2D/Box2D/Box2D.h"

#define GRAVITY_X 0.0f
#define GRAVITY_Y -10.0f

#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

// types of bodies
enum class bodyType {
	DYNAMIC,
	STATIC,
	KINEMATIC
};

enum class ColliderType {
	PLAYER, 
	ITEM,
	PLATFORM, 
	UNKNOWN
	// ..
};

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	PhysBody() = default;

	~PhysBody() = default;

	void GetPosition(int& x, int& y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;

	int width= 0;
	int height = 0;
	b2Body* body = nullptr;
	Entity* listener = nullptr;
	ColliderType ctype = ColliderType::UNKNOWN;
};

// Module --------------------------------------
class Physics : public Module, public b2ContactListener
{
public:

	// Constructors & Destructors
	Physics();
	~Physics() final;

	// Main module steps
	bool Start() final;
	bool PreUpdate() final;
	bool PostUpdate() final;
	bool CleanUp() final;

	// Create basic physics objects
	PhysBody* CreateRectangle(int x, int y, int width, int height, bodyType type);
	PhysBody* CreateCircle(int x, int y, int radius, bodyType type);
	PhysBody* CreateRectangleSensor(int x, int y, int width, int height, bodyType type);
	PhysBody* CreateChain(int x, int y, const int* const points, int size, bodyType type);
	
	// b2ContactListener ---
	void BeginContact(b2Contact* contact) final;

	bodyType GetEnumFromStr(const std::string &s) const;

private:

	// Debug mode
	bool debug = false;

	// Box2D World
	b2World* world = nullptr;

	static const std::unordered_map<std::string, bodyType> bodyTypeStrToEnum;
};