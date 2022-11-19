#include "Physics.h"
#include "Input.h"
#include "App.h"
#include "Log.h"
#include "math.h"
#include "SDL/include/SDL_keycode.h"
#include "Defs.h"
#include "Log.h"
#include "Render.h"
#include "Ball.h"
#include "Window.h"
#include "Box2D/Box2D/Box2D.h"

// Tell the compiler to reference the compiled Box2D libraries
#ifdef _DEBUG
#pragma comment( lib, "../Game/Source/External/Box2D/libx86/DebugLib/Box2D.lib" )
#else
#pragma comment( lib, "../Game/Source/External/Box2D/libx86/ReleaseLib/Box2D.lib" )
#endif

const std::unordered_map<std::string, bodyType> Physics::bodyTypeStrToEnum {
	{"dynamic", bodyType::DYNAMIC},
	{"static", bodyType::STATIC},
	{"kinematic", bodyType::KINEMATIC}
};

Physics::Physics() : Module()
{
}

// Destructor
Physics::~Physics() = default;

bool Physics::Start()
{
	LOG("Creating Physics 2D environment");

	// Create a new World
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));

	// Set this module as a listener for contacts
	world->SetContactListener(this);

	return true;
}

// 
bool Physics::PreUpdate()
{
	// Step (update) the World
	world->Step(1.0f / 60.0f, 6, 2);

	// Because Box2D does not automatically broadcast collisions/contacts with sensors, 
	// we have to manually search for collisions and "call" the equivalent to the ModulePhysics::BeginContact() ourselves...
	for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		// For each contact detected by Box2D, see if the first one colliding is a sensor
		if (c->IsTouching() && c->GetFixtureA()->IsSensor())
		{
			// If so, we call the OnCollision listener function (only of the sensor), passing as inputs our custom PhysBody classes
			auto* pb1 = (PhysBody*)c->GetFixtureA()->GetBody()->GetUserData();
			auto* pb2 = (PhysBody*)c->GetFixtureB()->GetBody()->GetUserData();
			
			if (pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

	return true;
}

PhysBody* Physics::CreateRectangle(int x, int y, int width, int height, bodyType type)
{
	b2BodyDef body;

	if(type == bodyType::DYNAMIC) body.type = b2_dynamicBody;
	if(type == bodyType::STATIC) body.type = b2_staticBody;
	if(type == bodyType::KINEMATIC) body.type = b2_kinematicBody;

	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	b->ResetMassData();

	b->CreateFixture(&fixture);

	auto* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width * 0.5f;
	pbody->height = height * 0.5f;

	return pbody;
}

PhysBody* Physics::CreateCircle(int x, int y, int radius, bodyType type)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case bodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case bodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case bodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body* b = world->CreateBody(&body);

	// Create SHAPE
	b2CircleShape circle;
	circle.m_radius = PIXEL_TO_METERS(radius);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 1.0f;
	b->ResetMassData();

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	auto* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = radius * 0.5f;
	pbody->height = radius * 0.5f;

	// Return our PhysBody class
	return pbody;
}

PhysBody* Physics::CreateRectangleSensor(int x, int y, int width, int height, bodyType type)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case bodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case bodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case bodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body* b = world->CreateBody(&body);

	// Create SHAPE
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.isSensor = true;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	auto* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width;
	pbody->height = height;

	// Return our PhysBody class
	return pbody;
}

PhysBody* Physics::CreateChain(int x, int y, const int* const points, int size, bodyType type)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case bodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case bodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case bodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body* b = world->CreateBody(&body);

	// Create SHAPE
	b2ChainShape shape;
	auto* p = new b2Vec2[size / 2];
	for (uint i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}
	shape.CreateLoop(p, size / 2);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &shape;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Clean-up temp array
	delete[] p;

	// Create our custom PhysBody class
	auto* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = pbody->height = 0;

	// Return our PhysBody class
	return pbody;
}

// 
bool Physics::PostUpdate()
{
	// Activate or deactivate debug mode
	if(app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(!debug)
		return true;

	//  Iterate all objects in the world and draw the bodies
	for(b2Body *b = world->GetBodyList(); b; b = b->GetNext())
	{
		for(b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext())
		{
			switch(f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::e_circle:
				{
					b2CircleShape const *shape = (b2CircleShape *)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					app->render->DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(shape->m_radius), 255, 255, 255);
				}
				break;

				// Draw polygons ------------------------------------------------
				case b2Shape::e_polygon:
				{
					b2PolygonShape const *itemToDraw = (b2PolygonShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, 255, 100, 100);
				}
				break;

				// Draw chains contour -------------------------------------------
				case b2Shape::e_chain:
				{
					b2ChainShape const *itemToDraw = (b2ChainShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices , 100, 255, 100);
				}
				break;

				// Draw a single segment(edge) ----------------------------------
				case b2Shape::e_edge:
				{
					b2EdgeShape const *shape = (b2EdgeShape *)f->GetShape();
					b2Vec2 v1;
					b2Vec2 v2;

					v1 = b->GetWorldPoint(shape->m_vertex0);
					v1 = b->GetWorldPoint(shape->m_vertex1);
					app->render->DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), 100, 100, 255);
				}
				break;

				case b2Shape::e_typeCount:
					//Info parameter. A shape should never have this type.
					break;

			}
		}
	}
	return true;
}

void Physics::DrawDebug(const b2Body *body, const int32 count, const b2Vec2 *vertices, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
{
	b2Vec2 prev = body->GetWorldPoint(vertices[0]);
	b2Vec2 v;

	for(int32 i = 1; i < count; ++i)
	{
		v = body->GetWorldPoint(vertices[i]);
		app->render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), r, g, b, a);
		prev = v;
	}

	v = body->GetWorldPoint(vertices[0]);
	app->render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), r, g, b, a);
}


// Called before quitting
bool Physics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	RELEASE(world)

	return true;
}

// Callback function to collisions with Box2D
void Physics::BeginContact(b2Contact* contact)
{
	// Call the OnCollision listener function to bodies A and B, passing as inputs our custom PhysBody classes
	auto* pBodyA = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData();
	auto* pBodyB = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData();

	if (pBodyA && pBodyA->listener)
		pBodyA->listener->OnCollision(pBodyA, pBodyB);

	if (pBodyB && pBodyB->listener)
		pBodyB->listener->OnCollision(pBodyB, pBodyA);
}

bodyType Physics::GetEnumFromStr(const std::string &s) const
{
	return bodyTypeStrToEnum.at(s);
}

//--------------- PhysBody

void PhysBody::GetPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x) - width;
	y = METERS_TO_PIXELS(pos.y) - height;
}

float PhysBody::GetRotation() const
{
	return RADTODEG * body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	for(const b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		//if point P is inside the fixture shape
		if(fixture->GetShape()->TestPoint(body->GetTransform(), p)) return true;
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;


	for(const b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0))
		{
			// do we want the normal ?
			float fx = x2 - x1;
			float fy = y2 - y1;
			float dist = sqrtf((fx * fx) + (fy * fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return output.fraction * dist;
		}
	}
	return -1;
}