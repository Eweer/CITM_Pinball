
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>
#include <unordered_map>
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

    mapFileName = config.child("mapfile").attribute("path").as_string();
    mapFolder = config.child("mapfolder").attribute("path").as_string();

    return ret;
}

void Map::Draw()
{
    if(!mapLoaded) return;

    ;

    for(ListItem<MapLayer *> *mapLayerItem = mapData.maplayers.start; mapLayerItem; mapLayerItem = mapLayerItem->next)
    {
        //ask each layer if your “Draw” property is true.
        Properties::Property const *bDataDrawable = mapLayerItem->data->properties.GetProperty("Draw");
        bool bDataDrawableHasValue = bDataDrawable->value;
        if(!bDataDrawable || !bDataDrawableHasValue) continue;

        for (int x = 0; x < mapLayerItem->data->width; x++)
        {
            for (int y = 0; y < mapLayerItem->data->height; y++)
            {
                int gid = mapLayerItem->data->Get(x, y);

                TileSet* tileset = GetTilesetFromTileId(gid);

                SDL_Rect r = tileset->GetTileRect(gid);
                iPoint pos = MapToWorld(x, y);

                app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
            }
        }
    }
}

iPoint Map::MapToWorld(int x, int y) const
{
    iPoint ret (0,0);

    ret.x = x * mapData.tileWidth;
    ret.y = y * mapData.tileHeight;

    return ret;
}

iPoint Map::WorldToMap(int x, int y) const
{
    iPoint ret(0, 0);

    ret.x = x/mapData.tileWidth;
    ret.y = y/mapData.tileHeight;

    return ret;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int gid) const
{
    SDL_Rect rect = { 0 };
    int relativeIndex = gid - firstgid;

    rect.w = tileWidth;
    rect.h = tileHeight;
    rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
    rect.y = margin + (tileWidth + spacing) * (relativeIndex / columns);

    return rect;
}


TileSet* Map::GetTilesetFromTileId(int gid) const
{
    ListItem<TileSet*>* item = mapData.tilesets.start;
    TileSet* set = nullptr;

    while (item)
    {
        set = item->data;
        if (gid < (item->data->firstgid + item->data->tilecount)) break;
        item = item->next;
    }

    return set;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

	ListItem<TileSet*>* item;
	item = mapData.tilesets.start;

	while (item)
	{
		RELEASE(item->data)
		item = item->next;
	}
	mapData.tilesets.Clear();

    ListItem<MapLayer*>* layerItem;
    layerItem = mapData.maplayers.start;

    while (layerItem)
    {
        RELEASE(layerItem->data)
        layerItem = layerItem->next;
    }

    return true;
}

// Load new map
bool Map::Load()
{
    mapLoaded = false;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapFileName.GetString());

    if(result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", mapFileName, result.description());
        return false;
    }

    if(!LoadMap(mapFileXML)) return false;
    if(!LoadTileSet(mapFileXML)) return false;
    if(!LoadAllLayers(mapFileXML.child("map"))) return false;
       
    
    // L07 DONE 3: Create colliders
    // Later you can create a function here to load and create the colliders from the map

    /*
    PhysBody* c1 = app->physics->CreateRectangle(224 + 128, 543 + 32, 256, 64, STATIC);
    // L07 DONE 7: Assign collider type
    c1->ctype = ColliderType::PLATFORM;

    PhysBody* c2 = app->physics->CreateRectangle(352 + 64, 384 + 32, 128, 64, STATIC);
    // L07 DONE 7: Assign collider type
    c2->ctype = ColliderType::PLATFORM;

    PhysBody* c3 = app->physics->CreateRectangle(256, 704 + 32, 576, 64, STATIC);
    // L07 DONE 7: Assign collider type
    c3->ctype = ColliderType::PLATFORM;
    */

    //LOG all the data loaded iterate all tilesets and LOG everything
       
    LOG("Successfully parsed map XML file :%s", mapFileName.GetString());
    LOG("width : %d height : %d",mapData.width,mapData.height);
    LOG("tile_width : %d tile_height : %d",mapData.tileWidth, mapData.tileHeight);
        
    LOG("Tilesets----");

    ListItem<TileSet*>* tileset;
    tileset = mapData.tilesets.start;

    while (tileset) {
        LOG("name : %s firstgid : %d",tileset->data->name.GetString(), tileset->data->firstgid);
        LOG("tile width : %d tile height : %d", tileset->data->tileWidth, tileset->data->tileHeight);
        LOG("spacing : %d margin : %d", tileset->data->spacing, tileset->data->margin);
        tileset = tileset->next;
    }

    ListItem<MapLayer*>* mapLayer;
    mapLayer = mapData.maplayers.start;

    while (mapLayer) {
        LOG("id : %d name : %s", mapLayer->data->id, mapLayer->data->name.GetString());
        LOG("Layer width : %d Layer height : %d", mapLayer->data->width, mapLayer->data->height);
        mapLayer = mapLayer->next;
    }

    if(mapFileXML) mapFileXML.reset();

    mapLoaded = true;

    return mapLoaded;
}

bool Map::LoadMap(pugi::xml_node mapFile)
{
    pugi::xml_node map = mapFile.child("map");

    if (map == NULL)
    {
        LOG("Error parsing map xml file: Cannot find 'map' tag.");
        return false;
    }

    //Load map general properties
    mapData.height = map.attribute("height").as_int();
    mapData.width = map.attribute("width").as_int();
    mapData.tileHeight = map.attribute("tileheight").as_int();
    mapData.tileWidth = map.attribute("tilewidth").as_int();
    std::string orientation = map.attribute("orientation").as_string();

    std::unordered_map<std::string, MapTypes> orientationRefMap {
        { "unknown", MapTypes::MAPTYPE_UNKNOWN },
        { "orthogonal", MapTypes::MAPTYPE_ORTHOGONAL },
        { "isometric", MapTypes::MAPTYPE_ISOMETRIC },
        { "staggered", MapTypes::MAPTYPE_STAGGERED }
    };

    mapData.type = orientationRefMap[orientation];

    return true;
}

bool Map::LoadTileSet(pugi::xml_node mapFile)
{
    for (pugi::xml_node tileset = mapFile.child("map").child("tileset"); tileset; tileset = tileset.next_sibling("tileset"))
    {
        auto* set = new TileSet();

        set->name = tileset.attribute("name").as_string();
        set->firstgid = tileset.attribute("firstgid").as_int();
        set->margin = tileset.attribute("margin").as_int();
        set->spacing = tileset.attribute("spacing").as_int();
        set->tileWidth = tileset.attribute("tilewidth").as_int();
        set->tileHeight = tileset.attribute("tileheight").as_int();
        set->columns = tileset.attribute("columns").as_int();
        set->tilecount = tileset.attribute("tilecount").as_int();

        SString tmp("%s%s", mapFolder.GetString(), tileset.child("image").attribute("source").as_string());
        set->texture = app->tex->Load(tmp.GetString());

        mapData.tilesets.Add(set);
    }

    return true;
}

bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
    bool ret = true;

    //Load the attributes
    layer->id = node.attribute("id").as_int();
    layer->name = node.attribute("name").as_string();
    layer->width = node.attribute("width").as_int();
    layer->height = node.attribute("height").as_int();

    LoadProperties(node, layer->properties);

    //Reserve the memory for the data 
    layer->data = new uint[layer->width * layer->height];
    memset(layer->data, 0, layer->width * layer->height);

    //Iterate over all the tiles and assign the values
    int i = 0;
    for (pugi::xml_node tile = node.child("data").child("tile"); tile && ret; tile = tile.next_sibling("tile"))
    {
        layer->data[i] = tile.attribute("gid").as_uint();
        i++;
    }

    return ret;
}

// L05: DONE 4: Iterate all layers and load each of them
bool Map::LoadAllLayers(pugi::xml_node mapNode) 
{
    for (pugi::xml_node layerNode = mapNode.child("layer"); layerNode; layerNode = layerNode.next_sibling("layer"))
    {
        //Load the layer
        auto* mapLayer = new MapLayer();
        if(!LoadLayer(layerNode, mapLayer)) return false;
        mapData.maplayers.Add(mapLayer);
    }

    return true;
}

//Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(const pugi::xml_node& node, Properties& properties) const
{
    for (pugi::xml_node propertyNode = node.child("properties").child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
    {
        auto* p = new Properties::Property();
        p->name = propertyNode.attribute("name").as_string();
        p->value = propertyNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!

        properties.list.Add(p);
    }

    return true;
}


//Ask for the value of a custom property
Properties::Property* Properties::GetProperty(const char* name) const
{
    for(ListItem<Property*>* item = list.start; item; item = item->next)
    {
        if(item->data->name == name) 
            return item->data;
    }

    return nullptr;
}


