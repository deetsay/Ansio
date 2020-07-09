#pragma once

#include <array>

#include <SDL_opengl.h>

#include "merx.h"

namespace ansio {
	class Ansio {
	public:
		float zoom;

		int width;
	    int height;

		merx::Font *font;

		merx::Palette *palette;

		merx::Merx current;
		merx::Merx defaultMerx;

		merx::Merx *edit_area;

		Ansio(merx::Font *font, merx::Palette *palette, int width, int height);
		virtual ~Ansio();
	};
}
