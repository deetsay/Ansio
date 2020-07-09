#include "ansio.h"

#include <iostream>
#include <set>
#include <regex>

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

#include "merx.h"

namespace ansio {
	Ansio::Ansio(merx::Font *font, merx::Palette *palette, int width, int height) {

		this->zoom = 1.0f;

		this->font = font;
		this->palette = palette;

		this->width = width;
		this->height = height;

		edit_area = new merx::Merx[width*height];
		for (int i=0; i<width*height; i++) {
			edit_area[i].character = ' ';
			edit_area[i].fg_color = 15;
			edit_area[i].bg_color = 0;
		}

		current.character = 'a';
		current.fg_color = 15;
		current.bg_color = 0;
		defaultMerx.character = ' ';
		defaultMerx.fg_color = 15;
		defaultMerx.bg_color = 0;
	}

	Ansio::~Ansio() {
		if (edit_area != NULL) {
			delete [] edit_area;
			edit_area = NULL;
		}
	}
}
