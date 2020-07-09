#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <string>

#include "imgui.h"

namespace merx {

	class Font {
	private:
		GLuint surface_to_texture(SDL_Surface *surface);
	public:
		std::string name;
		ImVec2 lit;
		int width;
		int height;
		GLuint gl_texture = 0;
		bool scandoubler;

		void init(std::string name, const void *data, int size, bool scandoubler);
		~Font();
	};

	class Palette {
	private:
	public:
		std::vector<ImVec4> color;
		std::string name;

		ImVec4 get_color(int c);
		bool is_bold(int c);
		void push_back(int rgb);
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
