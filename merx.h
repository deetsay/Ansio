#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "imgui.h"

namespace merx {

	class Font {
	private:
		GLuint surface_to_texture(SDL_Surface *surface);
	public:
		int width;
		int height;
		GLuint gl_texture;

		Font(const void *data, int size);
		~Font();
	};

	class Merx {
	public:
		int character;
		int fg_color;
		int bg_color;
	};

	class Undo : public Merx {
	public:
		int x;
		int y;
	};
}
