#ifndef __ModuleFonts_H__
#define __ModuleFonts_H__

#include "Module.h"
#include "SDL/include/SDL_pixels.h"

#define MAX_FONTS 10
#define MAX_FONT_CHARS 256

struct SDL_Texture;

struct Font
{
	char table[MAX_FONT_CHARS];
	SDL_Texture *graphic = nullptr;
	int rows;
	int len;
	int char_w;
	int char_h;
	int row_chars;
};

class Fonts : public Module
{
public:

	Fonts();
	~Fonts();

	// Load Font
	int Load(const char *texture_path, const char *characters, uint rows = 1);
	void UnLoad(int font_id);

	// Create a surface from text
	void Blit(int x, int y, int bmp_font_id, const char *text, int offsetY = 0, double angle = 5.0f) const;

private:

	Font	 fonts[MAX_FONTS];
};


#endif // __ModuleFonts_H__