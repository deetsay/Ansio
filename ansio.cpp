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
	Ansio::Ansio(merx::Font *font) {

		this->font = font;

		width = 80;
		height = 30;

		edit_area = new merx::Merx[width*height];
		for (int i=0; i<width*height; i++) {
			edit_area[i].character = ' ';
			edit_area[i].fg_color = 15;
			edit_area[i].bg_color = 0;
		}

		palette[0] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		palette[1] = ImVec4(0.73f, 0.0f, 0.0f, 1.0f);
		palette[2] = ImVec4(0.0f, 0.73f, 0.0f, 1.0f);
		palette[3] = ImVec4(0.73f, 0.73f, 0.0f, 1.0f);
		palette[4] = ImVec4(0.0f, 0.0f, 0.73f, 1.0f);
		palette[5] = ImVec4(0.73f, 0.0f, 0.73f, 1.0f);
		palette[6] = ImVec4(0.0f, 0.73f, 0.73f, 1.0f);
		palette[7] = ImVec4(0.73f, 0.73f, 0.73f, 1.0f);
		palette[8] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
		palette[9] = ImVec4(1.0f, 0.33f, 0.33f, 1.0f);
		palette[10] = ImVec4(0.33f, 1.0f, 0.33f, 1.0f);
		palette[11] = ImVec4(1.0f, 1.0f, 0.33f, 1.0f);
		palette[12] = ImVec4(0.33f, 0.33f, 1.0f, 1.0f);
		palette[13] = ImVec4(1.0f, 0.33f, 1.0f, 1.0f);
		palette[14] = ImVec4(0.33f, 1.0f, 1.0f, 1.0f);
		palette[15] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

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
