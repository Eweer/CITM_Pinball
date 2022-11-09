#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#include "Module.h"

#include "List.h"

struct SDL_Texture;
struct SDL_Surface;

class Textures : public Module
{
public:

	Textures();

	// Destructor
	virtual ~Textures();

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called before the first frame
	bool Start() final;

	// Called before quitting
	bool CleanUp() final;

	// Load Texture
	SDL_Texture* Load(const char* path) ;
	SDL_Texture* LoadSurface(SDL_Surface* surface);
	bool UnLoad(const SDL_Texture* texture);
	void GetSize(SDL_Texture* texture, uint& width, uint& height) const;

	List<SDL_Texture*> textures;
};


#endif // __TEXTURES_H__