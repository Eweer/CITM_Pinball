#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Textures.h"
#include "Render.h"
#include "Fonts.h"

#include<string.h>

// Constructor
Fonts::Fonts() : Module()
{
}

// Destructor
Fonts::~Fonts()
{
}

// Load new texture from file path
int Fonts::Load(const char *texture_path, const char *characters, uint rows)
{
	int id = -1;

	if(texture_path == nullptr || characters == nullptr || rows == 0)
	{
		LOG("Could not load font");
		return id;
	}

	SDL_Texture *tex = app->tex->Load(texture_path);

	if(tex == nullptr || strlen(characters) >= MAX_FONT_CHARS)
	{
		LOG("Could not load font at %s with characters '%s'", texture_path, characters);
		return id;
	}

	id = 0;
	for(; id < MAX_FONTS; ++id)
		if(fonts[id].graphic == nullptr)
			break;

	if(id == MAX_FONTS)
	{
		LOG("Cannot load font %s. Array is full (max %d).", texture_path, MAX_FONTS);
		return id;
	}

	fonts[id].graphic = app->tex->Load(texture_path);
	for(fonts[id].len = 0; *(characters + fonts[id].len) != 0; fonts[id].len++)
	{
		fonts[id].table[fonts[id].len] = *(characters + fonts[id].len);
	}

	fonts[id].rows = rows;
	uint width, height;
	app->tex->GetSize(fonts[id].graphic, width, height);
	fonts[id].row_chars = fonts[id].len / rows;
	fonts[id].char_h = height / rows;
	fonts[id].char_w = width / fonts[id].row_chars;

	LOG("Successfully loaded BMP font from %s", texture_path);

	return id;
}

void Fonts::UnLoad(int font_id)
{
	if(font_id >= 0 && font_id < MAX_FONTS && fonts[font_id].graphic != nullptr)
	{
		app->tex->UnLoad(fonts[font_id].graphic);
		fonts[font_id].graphic = nullptr;
		LOG("Successfully Unloaded BMP font_id %d", font_id);
	}
}

// Render text using a bmp font
void Fonts::Blit(int x, int y, int font_id, const char *text, int offsetY, double angle) const
{
	if(text == nullptr || font_id < 0 || font_id >= MAX_FONTS || fonts[font_id].graphic == nullptr)
	{
		LOG("Unable to render text with bmp font id %d", font_id);
		return;
	}

	const Font *font = &fonts[font_id];
	SDL_Rect rect;
	uint len = strlen(text);

	rect.w = font->char_w;
	rect.h = font->char_h;

	for(uint i = 0; i < len; ++i)
	{
		uint pos = 0;
		for(; pos < font->len; pos++)
		{
			if(*(text + i) == font->table[pos]) break;
		}

		uint c_row = pos / font->row_chars;
		uint c_col = pos % font->row_chars;

		rect.x = c_col*font->char_w;
		rect.y = c_row*font->char_h;

		app->render->DrawTexture(font->graphic, x + i*font->char_w, y + i*offsetY, &rect, 1.0f, angle);
	}
}
