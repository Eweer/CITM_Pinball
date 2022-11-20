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

	std::string musicPath;
	std::string texturePath;

	uint backgroundMusic = 0;

	SDL_Texture *backgroundImage = nullptr;
};

#endif // __MAP_H__