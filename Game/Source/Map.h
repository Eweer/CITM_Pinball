#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "List.h"
#include "Point.h"
#include "Physics.h"

#include "PugiXml\src\pugixml.hpp"

class Map : public Module
{
public:

    Map();

    // Destructor
    virtual ~Map();

    // Called before render is available
    bool Awake(pugi::xml_node& conf) final;

    // Called each loop iteration
    void Draw();

    // Called before quitting
    bool CleanUp() final;

    // Load new map
    bool Load();

private:

    void CreateCollidersBasedOnName(const pugi::xml_node &colliderAttributes, bodyType typeOfChildren);
    void CreateChainColliders(const std::string &xyStr, bodyType bodyT);


    pugi::xml_document collidersFile;
	SDL_Texture *background = nullptr;
	std::string mapFolder;
	uint levelNumber = 1;
};

#endif // __MAP_H__