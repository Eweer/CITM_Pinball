#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "List.h"
#include "Point.h"

#include "PugiXml\src\pugixml.hpp"

// Ignore Terrain Types and Tile Types for now, but we want the image!
struct TileSet
{
	SString	name;
	int	firstgid;
	int margin;
	int	spacing;
	int	tileWidth;
	int	tileHeight;
	int columns;
	int tilecount;

	SDL_Texture* texture;

	SDL_Rect GetTileRect(int gid) const;
};

//  We create an enum for map type, just for convenience,
enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};

struct Properties
{
	struct Property
	{
		SString name = nullptr;
		bool value = false;
	};

	~Properties()
	{
		list.Clear();
	}

	Property* GetProperty(const char* name) const;

	List<Property*> list;
};

struct MapLayer
{
	SString	name = nullptr;
	int id = -1;
	int width = 0;
	int height = 0;
	uint *data = nullptr;

	Properties properties;

	~MapLayer()
	{
		RELEASE(data)
	}

	inline uint Get(int x, int y) const
	{
		return data[(y * width) + x];
	}
};

struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	List<TileSet*> tilesets;
	MapTypes type;

	List<MapLayer*> maplayers;
};

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

	// Translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;

	// WorldToMap to obtain  
	iPoint Map::WorldToMap(int x, int y) const;

private:

	bool LoadMap(pugi::xml_node mapFile);

	bool LoadTileSet(pugi::xml_node mapFile);

	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);
	bool LoadAllLayers(pugi::xml_node mapNode);

	TileSet* GetTilesetFromTileId(int gid) const;

	bool LoadProperties(const pugi::xml_node& node, Properties& properties) const;

public: 

	MapData mapData;

private:

    SString mapFileName;
	SString mapFolder;
    bool mapLoaded = false;
};

#endif // __MAP_H__