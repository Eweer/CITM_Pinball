
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>
#include "SDL_image/include/SDL_image.h"

Map::Map() : Module()
{
    name.Create("map");
}

// Destructor
Map::~Map() = default;

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
    LOG("Loading Map Parser");
    bool ret = true;

    mapFolder = config.child("mapfolder").attribute("path").as_string();

    return ret;
}

void Map::Draw()
{
    app->render->DrawTexture(background, 0, 0);
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    return true;
}

// Load new map
bool Map::Load()
{
    auto levelFileName = mapFolder + "bg_" + std::to_string(levelNumber) + ".png";

    background = app->tex->Load(levelFileName.c_str());

    // L07 DONE 3: Create colliders
    // Later you can create a function here to load and create the colliders from the map

    PhysBody* c1 = app->physics->CreateRectangle(224 + 128, 543 + 32, 256, 64, bodyType::STATIC);
    c1->ctype = ColliderType::PLATFORM;

    PhysBody* c2 = app->physics->CreateRectangle(352 + 64, 384 + 32, 128, 64, bodyType::STATIC);
    c2->ctype = ColliderType::PLATFORM;

    PhysBody* c3 = app->physics->CreateRectangle(256, 704 + 32, 576, 64, bodyType::STATIC);
    c3->ctype = ColliderType::PLATFORM;


    return true;
}