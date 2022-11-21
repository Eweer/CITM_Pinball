#include "App.h"
#include "Window.h"
#include "Render.h"
#include "Input.h"

#include "Defs.h"
#include "Log.h"

#define VSYNC false

Render::Render() : Module()
{
	name.Create("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render() = default;

// Called before render is available
bool Render::Awake(pugi::xml_node& config)
{
	vSyncMode = config.child("vsync").attribute("value").as_bool();
	vSyncOnRestart = vSyncMode;

	LOG("Create SDL rendering context");

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if(vSyncMode)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(app->win->window, -1, flags);

	if(!renderer)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	camera.w = app->win->screenSurface->w;
	camera.h = app->win->screenSurface->h;
	camera.x = 0;
	camera.y = 0;

	ticksForNextFrame = 1000 / fpsTarget;
	
	return true;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	if(app->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN)
	{
		vSyncOnRestart = !vSyncOnRestart;
		app->SaveToConfig(name.GetString(), "vsync", "value", vSyncOnRestart ? "true" : "false");
	}
	if(!vSyncMode)
	{
		while(SDL_GetTicks() - lastTime < ticksForNextFrame)
		{
			SDL_Delay(1);
		}
	}
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	if (app->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN && fpsTarget < 1000)
	{
		fpsTarget += 10; 
		ticksForNextFrame = 1000 / fpsTarget;
	}
	if (app->input->GetKey(SDL_SCANCODE_O) == KEY_DOWN && fpsTarget > 10)
	{
		fpsTarget -= 10;
		ticksForNextFrame = 1000 / fpsTarget;
	}
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	
	if(!vSyncMode) lastTime = SDL_GetTicks();

	fpsFrames++;
	if (fpsLastTime < (SDL_GetTicks() - FPS_INTERVAL * 1000))
	{
		fpsLastTime = SDL_GetTicks();
		fpsCurrent = fpsFrames;
		fpsFrames = 0;
	}

	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
	uint scale = app->win->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if(section)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point const* p = nullptr;
	SDL_Point pivot{};

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result == -1)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result == -1)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360]{};

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = (int)(camera.x + x + radius * cos(i * factor));
		points[i].y = (int)(camera.y + y + radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result == -1)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::LoadState(pugi::xml_node& data)
{
	camera.x = data.child("camera").attribute("x").as_int();
	camera.y = data.child("camera").attribute("y").as_int();

	return true;
}

bool Render::SaveState(pugi::xml_node& data)
{
	pugi::xml_node cam = data.append_child("camera");

	cam.append_attribute("x") = camera.x;
	cam.append_attribute("y") = camera.y;

	pugi::xml_node vsyncNode = data.append_child("vsync");

	vsyncNode.append_attribute("active") = vSyncOnRestart;

	return true;
}

uint Render::GetCurrentFPS() const
{
	return fpsCurrent;
}

uint Render::GetTargetFPS() const
{
	return fpsTarget;
}

bool Render::IsVSyncActive() const
{
	return vSyncMode;
}

bool Render::RestartForVSync() const
{
	return vSyncMode != vSyncOnRestart;
}
