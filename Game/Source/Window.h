#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Module.h"

struct SDL_Window;
struct SDL_Surface;

class Window : public Module
{
public:

	Window();

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called before quitting
	bool CleanUp() final;

	// Changae title
	void SetTitle(const char* title);

	// Retrive window size
	void GetWindowSize(uint& w, uint& h) const;

	// Retrieve window scale
	uint GetScale() const;

	// The window we'll be rendering to
	SDL_Window* window = nullptr;

	// The surface contained by the window
	SDL_Surface* screenSurface = nullptr;

private:
	SString title = nullptr;
	uint width = 0;
	uint height = 0;
	uint scale = 1;
};

#endif // __WINDOW_H__