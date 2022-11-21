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

	// Called before the first frame
	bool Start() final;

	bool PostUpdate() final;

	// Called each loop iteration
	void Draw();

	// Called before quitting
	bool CleanUp() final;

	// Load new map
	bool Load();


	
	void DrawUI() const;

	void DrawGravity(int x, int y) const;
	void DrawFPS(int x, int y) const;
	void DrawScores(int x, int y, int offsetY, double angle) const;

	
	//void OffsetDrawPosition(iPoint &position, iPoint amount, bool condition = true, std::string const &str = "", uint font = 0);


private:

	std::string musicPath;
	std::string texturePath;
	std::string fontsPath;

	uint fontWhite = 0;
	uint fontOrange = 0;

	uint backgroundMusic = 0;

	SDL_Texture *boardImage = nullptr;
	SDL_Texture *backgroundImage = nullptr;
};

#endif // __MAP_H__