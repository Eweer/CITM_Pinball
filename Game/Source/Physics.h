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
enum class BodyType {
	DYNAMIC,
	STATIC,
	KINEMATIC,
	UNKNOWN
};

enum class ColliderType {
	BALL, 
	ITEM,
	ANIM, 
	UNKNOWN
	// ..
};

enum class Layers
{
	LAUNCH = 0, 
	BOARD, 
	KICKERS, 
	BALL, 
	TOP
};

enum class RevoluteJoinTypes
{
	 IPOINT,
	 BOOL,
	 FLOAT,
	 INT,
	 UNKNOWN
};

struct RevoluteJointSingleProperty
{
	RevoluteJoinTypes type;
	union
	{
		iPoint p;
		bool b;
		float f;
		int i;
	};

	RevoluteJointSingleProperty::RevoluteJointSingleProperty() {};
	RevoluteJointSingleProperty::RevoluteJointSingleProperty(const RevoluteJointSingleProperty &r) : type(r.type)
	{
		switch(type)
		{
			case RevoluteJoinTypes::BOOL:
				b = r.b;
				break;

			case RevoluteJoinTypes::FLOAT:
				f = r.f;
				break;

			case RevoluteJoinTypes::INT:
				i = r.i;
				break;

			case RevoluteJoinTypes::IPOINT:
				p = r.p;
				break;
			case RevoluteJoinTypes::UNKNOWN:
				LOG("Something went wrong in InteractiveParts doing the revolute joint");
				break;
		}
	};
	RevoluteJointSingleProperty::~RevoluteJointSingleProperty() {};
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
	PhysBody* CreateRectangle(int x, int y, int width, int height, BodyType type);
	PhysBody* CreateCircle(int x, int y, int radius, BodyType type, float rest = 0.0f, int cat = (int)Layers::BOARD, int mask = (int)Layers::BALL);
	PhysBody* CreateRectangleSensor(int x, int y, int width, int height, BodyType type);
	PhysBody* CreateChain(int x, int y, const int* const points, int size, BodyType type, float rest = 0.0f, int cat = (int)Layers::BOARD, int mask = (int)Layers::BALL, int angle = 0);

	// Create joints
	b2RevoluteJoint *CreateRevoluteJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties);
	b2MouseJoint *CreateMouseJoint(PhysBody *ground, PhysBody *target, b2Vec2 position, float dampingRatio = 0.5f, float frequecyHz = 2.0f, float maxForce = 100.0f);
	b2MouseJoint *CreateMouseJoint(b2Body *ground, b2Body *target, b2Vec2 position, float dampingRatio = 0.5f, float frequecyHz = 2.0f, float maxForce = 100.0f);
	
	// b2ContactListener ---
	void BeginContact(b2Contact* contact) final;

	// Utils
	iPoint WorldVecToIPoint(const b2Vec2 &v) const;
	b2Vec2 IPointToWorldVec(const iPoint &p) const;

	// Get Info
	bool IsDebugActive() const;
	BodyType GetEnumFromStr(const std::string &s) const;
	RevoluteJoinTypes GetTypeFromProperty(const std::string &s) const;

private:

	// Debug
	void DrawDebug(const b2Body *body, const int32 count, const b2Vec2 *vertices, Uint8 r, Uint8 g, Uint8 b, Uint8 a = (Uint8)255U) const;

	// Joints
	void DragSelectedObject();
	bool IsMouseOverObject(b2Fixture const *f) const;

	// Debug mode
	bool debug = false;
	bool debugWhileSelected = true;
	bool stepActive = true;

	// Box2D World
	b2World* world = nullptr;
	b2Body *ground;

	// Mouse Joint
	b2Body *selected = nullptr;
	b2MouseJoint *mouseJoint = nullptr;

	static const std::unordered_map<std::string, BodyType> bodyTypeStrToEnum;
	static const std::unordered_map<std::string, RevoluteJoinTypes> propertyToType;
};