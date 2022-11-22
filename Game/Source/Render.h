#ifndef __RENDER_H__
#define __RENDER_H__

#include "Module.h"

#include "Point.h"

#include "PugiXml/src/pugixml.hpp"
#include "SDL/include/SDL.h"

class Render : public Module
{
public:

	Render();

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called before the first frame
	bool Start() final;

	// Called each loop iteration
	bool PreUpdate() final;
	bool Update(float dt) final;
	bool PostUpdate() final;

	// Called before quitting
	bool CleanUp() final;

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX, SDL_RendererFlip flip = SDL_FLIP_NONE) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	bool LoadState(pugi::xml_node const &) override final;
	bool SaveState(pugi::xml_node const &) override final;

	uint GetCurrentFPS() const;
	uint GetTargetFPS() const;
	bool IsVSyncActive() const;
	bool RestartForVSync() const;

	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;


private:
	bool vSyncOnRestart = 0;
	bool vSyncMode = 0;

	uint fpsLastTime = 0;
	uint fpsCurrent = 0;
	uint fpsFrames = 0;

	uint lastTime = 0;
	uint fpsTarget = 60;
	uint ticksForNextFrame = 1000;
};

#endif // __RENDER_H__