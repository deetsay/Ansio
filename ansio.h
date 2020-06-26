#pragma once

#include <array>

#include <SDL_opengl.h>

#include "merx.h"

namespace ansio {
	class Ansio {
	public:
		int width;
	    int height;

		//merx::Scii collection[256];
		merx::Font *font;

		ImVec4 palette[16];
		merx::Merx current;
		merx::Merx defaultMerx;

		merx::Merx *edit_area;

		Ansio(merx::Font *font);
		virtual ~Ansio();
	};
}
