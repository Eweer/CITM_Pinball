#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "List.h"
#include "Point.h"

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

    void CreateChainColliders(const std::string &xyStr);

    pugi::xml_document collidersFile;
	SDL_Texture *background = nullptr;
	std::string mapFolder;
	uint levelNumber = 1;
};

#endif // __MAP_H__