#include "App.h"
#include "Render.h"
#include "Textures.h"

#include "Defs.h"
#include "Log.h"

#include "SDL_image/include/SDL_image.h"
//#pragma comment(lib, "../Game/Source/External/SDL_image/libx86/SDL2_image.lib")

Textures::Textures() : Module()
{
	name.Create("textures");
}

// Destructor
Textures::~Textures() = default;

// Called before render is available
bool Textures::Awake(pugi::xml_node& config)
{
	LOG("Init Image library");

	// Load support for the PNG image format
	int flags = IMG_INIT_PNG;
	int init = IMG_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Image lib. IMG_Init: %s", IMG_GetError());
		return false;
	}

	return true;
}

// Called before the first frame
bool Textures::Start()
{
	LOG("start textures");

	return true;
}

// Called before quitting
bool Textures::CleanUp()
{
	LOG("Freeing textures and Image library");

	for(ListItem<SDL_Texture*>* item = textures.start; item; item = item->next)
	{
		SDL_DestroyTexture(item->data);
	}

	textures.Clear();
	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture* Textures::Load(const char* path) 
{
	SDL_Surface* surface = IMG_Load(path);

	if(!surface)
	{
		LOG("Could not load surface with path: %s. IMG_Load: %s", path, IMG_GetError());
		return nullptr;
	}

	SDL_Texture* texture = LoadSurface(surface);
	SDL_FreeSurface(surface);

	return texture;
}

// Unload texture
bool Textures::UnLoad(const SDL_Texture* texture)
{

	for(ListItem<SDL_Texture*>* item = textures.start; item; item = item->next)
	{
		if(texture == item->data)
		{
			SDL_DestroyTexture(item->data);
			textures.Del(item);
			return true;
		}
	}

	return false;
}

// Translate a surface into a texture
SDL_Texture* Textures::LoadSurface(SDL_Surface* surface)
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(app->render->renderer, surface);

	if(!texture)
	{
		LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
		return nullptr;
	}

	textures.Add(texture);

	return texture;
}

// Retrieve size of a texture
void Textures::GetSize(SDL_Texture* texture, uint& width, uint& height) const
{
	SDL_QueryTexture(texture, nullptr, nullptr, (int*) &width, (int*) &height);
}
