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
		ImVec2 lit;
		int width;
		int height;
		GLuint gl_texture = 0;

		void init(const void *data, int size);
		~Font();
	};

	class Palette {
	private:
	public:
		ImVec4 color[16];
		int size;

		ImVec4 get_color(int c);
		bool is_bold(int c);
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
