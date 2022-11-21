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

const std::unordered_map<std::string, BodyType> Physics::bodyTypeStrToEnum {
	{"dynamic", BodyType::DYNAMIC},
	{"static", BodyType::STATIC},
	{"kinematic", BodyType::KINEMATIC},
	{"unknown", BodyType::UNKNOWN}
};

const std::unordered_map<std::string, RevoluteJoinTypes> Physics::propertyToType{
	{"anchor_offset", RevoluteJoinTypes::IPOINT},
	{"body_offset", RevoluteJoinTypes::IPOINT},
	{"enable_limit", RevoluteJoinTypes::BOOL},
	{"max_angle", RevoluteJoinTypes::FLOAT},
	{"min_angle", RevoluteJoinTypes::FLOAT},
	{"enable_motor", RevoluteJoinTypes::BOOL},
	{"motor_speed", RevoluteJoinTypes::INT},
	{"max_torque", RevoluteJoinTypes::INT}
};

Physics::Physics() : Module()
{
}

// Destructor
Physics::~Physics() = default;

//--------------- 

bool Physics::Start()
{
	LOG("Creating Physics 2D environment");

	// Create a new World
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));

	// Set this module as a listener for contacts
	world->SetContactListener(this);

	//Setting up so we can use joints
	b2BodyDef bd;
	ground = world->CreateBody(&bd);

	return true;
}

bool Physics::PreUpdate()
{
	float newGrav = b2_maxFloat;
	for (uint keyIterator = SDL_SCANCODE_1; keyIterator <= SDL_SCANCODE_0; keyIterator++)
	{
		if (app->input->GetKey(keyIterator) == KEY_DOWN) {
			if (keyIterator == SDL_SCANCODE_0)
			{
				newGrav = 0;
				break;
			}
			newGrav = ((int)keyIterator - (int)SDL_SCANCODE_1 + 1);
			if (app->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) newGrav *= -1;
			if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) newGrav *= 2;
		}
	}

	if (newGrav != b2_maxFloat)
	{
		b2Vec2 newGravVec;
		if (app->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
			newGravVec = { newGrav, world->GetGravity().y };
		else
			newGravVec = { world->GetGravity().x, newGrav };
		world->SetGravity(newGravVec);
	}

	// Step (update) the World
	if(stepActive || (!stepActive && app->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN))
	   world->Step(1.0f / 60.0f, 6, 2);
	
	if(app->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN) stepActive = !stepActive;

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

bool Physics::PostUpdate()
{
	// Activate or deactivate debug mode
	if(app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(app->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) 
		debugWhileSelected = !debugWhileSelected;

	if(!debug) return true;

	//  Iterate all objects in the world and draw the bodies
	//  until there are no more bodies or 
	//  we are dragging an object around and not debugging draw in the meantime
	for(b2Body *b = world->GetBodyList(); b && (!selected || (selected && debugWhileSelected)); b = b->GetNext())
	{
		for(b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext())
		{
			if(app->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && IsMouseOverObject(f))
			{
				selected = f->GetBody();
				break;
			}
			switch(f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::Type::e_circle:
				{
					auto const circleShape = (b2CircleShape *)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					app->render->DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(circleShape->m_radius), 255, 255, 255);
					break;
				}
				// Draw polygons ------------------------------------------------
				case b2Shape::Type::e_polygon:
				{
					auto const *itemToDraw = (b2PolygonShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, 255, 255, 0);
					break;
				}
				// Draw chains contour -------------------------------------------
				case b2Shape::Type::e_chain:
				{
					auto const *itemToDraw = (b2ChainShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, 100, 255, 100);
					break;
				}
				// Draw a single segment(edge) ----------------------------------
				case b2Shape::Type::e_edge:
				{
					auto const *edgeShape = (b2EdgeShape *)f->GetShape();
					b2Vec2 v1;
					b2Vec2 v2;

					v1 = b->GetWorldPoint(edgeShape->m_vertex0);
					v1 = b->GetWorldPoint(edgeShape->m_vertex1);
					app->render->DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), 100, 100, 255);
					break;
				}
				case b2Shape::Type::e_typeCount:
				{
					//Info parameter. A shape should never have this type.
					break;
				}
			}
		}
	}

	if(selected) DragSelectedObject();

	return true;
}


//--------------- Called before quitting

bool Physics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	RELEASE(world)

	return true;
}


//--------------- Callback function to collisions with Box2D

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


//--------------- Create Shapes and Joints

PhysBody *Physics::CreateRectangle(int x, int y, int width, int height, BodyType type, float32 gravityScale, float rest, uint16 cat, uint16 mask)
{
	b2BodyDef body;
	switch(type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("CreateRectangle Received UNKNOWN BodyType");
			return nullptr;
	}
	body.gravityScale = gravityScale;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body *b = world->CreateBody(&body);

	// Create SHAPE
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.restitution = rest;
	fixture.filter.categoryBits = cat;
	fixture.filter.maskBits = mask;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	auto *pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width * 0.5f;
	pbody->height = height * 0.5f;

	// Return our PhysBody class
	return pbody;
}

PhysBody *Physics::CreateCircle(int x, int y, int radius, BodyType type, float rest, uint16 cat, uint16 mask)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("CreateRectangle Received UNKNOWN BodyType");
			return nullptr;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body *b = world->CreateBody(&body);

	// Create SHAPE
	b2CircleShape circle;
	circle.m_radius = PIXEL_TO_METERS(radius);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = (uint16)cat;
	fixture.filter.maskBits = (uint16)mask;
	fixture.restitution = rest;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	auto *pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = radius * 0.5f;
	pbody->height = radius * 0.5f;

	// Return our PhysBody class
	return pbody;
}

PhysBody *Physics::CreatePolygon(int x, int y, const int* const points, int size, BodyType type, float rest, uint16 cat, uint16 mask, int angle)
{
	b2BodyDef body;
	switch(type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("Create Polygon Received UNKNOWN BodyType");
			return nullptr;
	}

	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.angle = DEGTORAD*(float)angle;

	b2Body *b = world->CreateBody(&body);
	b2PolygonShape box;
	b2Vec2 *p = new b2Vec2[size / 2];

	for(uint i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}
	box.Set(p, size / 2);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = cat;
	fixture.filter.maskBits = mask;
	fixture.restitution = rest;

	b->CreateFixture(&fixture);

	PhysBody *pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->height = pbody->width = 0;

	return pbody;
}

PhysBody *Physics::CreateRectangleSensor(int x, int y, int width, int height, BodyType type, uint16 cat, uint16 mask)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("CreateRectangle Received UNKNOWN BodyType");
			return nullptr;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body *b = world->CreateBody(&body);

	// Create SHAPE
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.isSensor = true;
	fixture.filter.categoryBits = cat;
	fixture.filter.maskBits = mask;

	b->CreateFixture(&fixture);


	auto *pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width;
	pbody->height = height;


	return pbody;
}

PhysBody *Physics::CreateChain(int x, int y, const int *const points, int size, BodyType type, float rest, uint16 cat, uint16 mask, int angle)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch(type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("CreateRectangle Received UNKNOWN BodyType");
			return nullptr;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.angle = DEGTORAD*(float)angle;

	// Add BODY to the world
	b2Body *b = world->CreateBody(&body);

	// Create SHAPE
	b2ChainShape shape;
	auto *p = new b2Vec2[size / 2];
	for(uint i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}
	shape.CreateLoop(p, size / 2);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = (uint16)cat;
	fixture.filter.maskBits = (uint16)mask;
	fixture.restitution = rest;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Clean-up temp array
	delete[] p;

	// Create our custom PhysBody class
	auto *pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = pbody->height = 0;

	// Return our PhysBody class
	return pbody;
}

b2RevoluteJoint *Physics::CreateRevoluteJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties)
{
	b2RevoluteJointDef rJoint;
	rJoint.bodyA = anchor->body;
	rJoint.bodyB = body->body;
	rJoint.collideConnected = false;

	rJoint.localAnchorA = b2Vec2(PIXEL_TO_METERS(anchorOffset.x), PIXEL_TO_METERS(anchorOffset.y));
	rJoint.localAnchorB = b2Vec2(PIXEL_TO_METERS(bodyOffset.x), PIXEL_TO_METERS(bodyOffset.y));

	if((rJoint.enableLimit = properties[0].b))
	{
		rJoint.upperAngle = DEGTORAD * (properties[1].f);
		rJoint.lowerAngle = DEGTORAD * (properties[2].f);
	}
	if((rJoint.enableMotor = properties[3].b))
	{
		rJoint.motorSpeed = (float)properties[4].i;
		rJoint.maxMotorTorque = (float)properties[5].i;
	}

	auto *returnJoint = ((b2RevoluteJoint *)world->CreateJoint(&rJoint));
	return returnJoint;
}

b2PrismaticJoint *Physics::CreatePrismaticJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties)
{
	b2PrismaticJointDef pJoint;
	pJoint.bodyA = anchor->body;
	pJoint.bodyB = body->body;
	pJoint.collideConnected = false;

	pJoint.localAnchorA = b2Vec2(PIXEL_TO_METERS(anchorOffset.x), PIXEL_TO_METERS(anchorOffset.y));
	pJoint.localAnchorB = b2Vec2(PIXEL_TO_METERS(bodyOffset.x), PIXEL_TO_METERS(bodyOffset.y));

	pJoint.localAxisA = b2Vec2(0, 1);

	if((pJoint.enableLimit = properties[0].b))
	{
		pJoint.lowerTranslation = DEGTORAD * (properties[1].f);
		pJoint.upperTranslation = DEGTORAD * (properties[2].f);
	}
	if((pJoint.enableMotor = properties[3].b))
	{
		pJoint.motorSpeed = (float)properties[4].i;
		pJoint.maxMotorForce = (float)properties[5].i;
	}

	return (b2PrismaticJoint *)world->CreateJoint(&pJoint);
}

b2MouseJoint *Physics::CreateMouseJoint(PhysBody *origin, PhysBody *target, b2Vec2 position, float dampingRatio, float frequecyHz, float maxForce)
{
	b2MouseJointDef mJointDef;
	mJointDef.bodyA = origin->body;
	mJointDef.bodyB = target->body;
	mJointDef.target = position;
	mJointDef.dampingRatio = dampingRatio;
	mJointDef.frequencyHz = frequecyHz;
	mJointDef.maxForce = maxForce * selected->GetMass();

	return ((b2MouseJoint *)world->CreateJoint(&mJointDef));
}

b2MouseJoint *Physics::CreateMouseJoint(b2Body *origin, b2Body *target, b2Vec2 position, float dampingRatio, float frequecyHz, float maxForce)
{
	b2MouseJointDef mJointDef;
	mJointDef.bodyA = origin;
	mJointDef.bodyB = target;
	mJointDef.target = position;
	mJointDef.dampingRatio = dampingRatio;
	mJointDef.frequencyHz = frequecyHz;
	mJointDef.maxForce = maxForce * selected->GetMass();

	return ((b2MouseJoint *)world->CreateJoint(&mJointDef));
}


//--------------- Utils

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

void Physics::DragSelectedObject()
{
	int mouseX;
	int mouseY;
	app->input->GetMousePosition(mouseX, mouseY);
	b2Vec2 target(PIXEL_TO_METERS(mouseX), PIXEL_TO_METERS(mouseY));

	switch(app->input->GetMouseButtonDown(SDL_BUTTON_LEFT))
	{
		case KeyState::KEY_DOWN:
		{
			mouseJoint = CreateMouseJoint(ground, selected, target);
			break;
		}
		case KeyState::KEY_REPEAT:
		{
			mouseJoint->SetTarget(target);
			app->render->DrawLine(mouseX, mouseY, METERS_TO_PIXELS(selected->GetPosition().x), METERS_TO_PIXELS(selected->GetPosition().y), 0, 255, 255, 255);
			break;
		}
		case KeyState::KEY_UP:
		{
			DestroyMouseJoint();
			break;
		}
		case KeyState::KEY_IDLE:
			break;

	}

}

bool Physics::IsMouseOverObject(b2Fixture const *f) const
{
	if(f->TestPoint(IPointToWorldVec(app->input->GetMousePosition())))
		return true;
	return false;
}

void Physics::DestroyMouseJoint()
{
	world->DestroyJoint(mouseJoint);
	if(mouseJoint) mouseJoint = nullptr;
	if(selected) selected = nullptr;
}

bool Physics::IsDebugActive() const
{
	return debug;
}

iPoint Physics::WorldVecToIPoint(const b2Vec2 &v) const
{
	return iPoint(METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y));
}

b2Vec2 Physics::IPointToWorldVec(const iPoint &p) const
{
	return b2Vec2(PIXEL_TO_METERS(p.x), PIXEL_TO_METERS(p.y));
}

b2Vec2 Physics::GetWorldGravity() const
{
	return world->GetGravity();
}

void Physics::DestroyBody(b2Body *b)
{
	if(b) world->DestroyBody(b);
}

void Physics::DestroyPhysBody(PhysBody *b)
{
	DestroyBody(b->body);
	if(b) delete b;

}

BodyType Physics::GetEnumFromStr(const std::string &s) const
{
	if(!bodyTypeStrToEnum.count(s))
	{
		LOG("Physics::GetEnumFromStr didn't find %s attribute.", s);
		return BodyType::UNKNOWN;
	}
	return bodyTypeStrToEnum.at(s);
}

RevoluteJoinTypes Physics::GetTypeFromProperty(const std::string &s) const
{
	if(!propertyToType.count(s))
	{
		LOG("Physics::GetTypeFromProperty didn't find %s attribute.", s);
		return RevoluteJoinTypes::UNKNOWN;
	}
	return propertyToType.at(s);
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