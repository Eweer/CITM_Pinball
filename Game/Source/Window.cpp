#include "Window.h"
#include "App.h"

#include "Defs.h"
#include "Log.h"

#include "SDL/include/SDL.h"


Window::Window() : Module()
{
	name.Create("window");
}

// Destructor
Window::~Window() = default;

// Called before render is available
bool Window::Awake(pugi::xml_node& config)
{
	LOG("Init SDL window & surface");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	Uint32 flags = SDL_WINDOW_SHOWN;
	bool fullscreen = config.child("fullscreen").attribute("value").as_bool(); 
	bool borderless = config.child("bordeless").attribute("value").as_bool();
	bool resizable = config.child("resizable").attribute("value").as_bool(); 
	bool fullscreen_window = config.child("fullscreen_window").attribute("value").as_bool();

	width = config.child("resolution").attribute("width").as_int(); 
	height = config.child("resolution").attribute("height").as_int();
	scale = config.child("resolution").attribute("scale").as_int();

	if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
	if (borderless) flags |= SDL_WINDOW_BORDERLESS;
	if (resizable) flags |= SDL_WINDOW_RESIZABLE;
	if (fullscreen_window) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	window = SDL_CreateWindow(app->GetTitle(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

	if (!window)
	{
		LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	screenSurface = SDL_GetWindowSurface(window);

	return true;
}

// Called before quitting
bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window) SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();
	return true;
}

// Set new window title
void Window::SetTitle(const char* new_title)
{
	SDL_SetWindowTitle(window, new_title);
}

void Window::GetWindowSize(uint& w, uint& h) const
{
	w = this->width;
	h = this->height;
}

uint Window::GetScale() const
{
	return scale;
}