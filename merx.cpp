#include "merx.h"
#include <iostream>

namespace merx {

	ImVec4 Palette::get_color(int c) {
		return color[c & (color.size()-1)];
	}

	bool Palette::is_bold(int c) {
		return color.size()<=8 && c>=8;
	}

	void Palette::push_back(int rgb) {
		color.push_back(
			ImVec4((float)((rgb>>16) & 0xff) / 255.0,
			(float)((rgb>>8) & 0xff) / 255.0,
			(float)(rgb & 0xff) / 255.0,
			1.0f));
	}

	GLuint Font::surface_to_texture(SDL_Surface *surface) {

		GLuint gltex = 0;
		SDL_Surface *copy_surface = SDL_CreateRGBSurfaceWithFormat(
			0, surface->w, surface->h, 32, SDL_PIXELFORMAT_ABGR8888);
		if (copy_surface == NULL) {
			SDL_FreeSurface(surface);
			return gltex;
		}

		unsigned char *ptr = (unsigned char *) copy_surface->pixels;
		int litx=-1;
		int lity=-1;
		int y=0;
		for (int i=0, x=0; i<surface->w*surface->h; i++, x++) {
			if (x==surface->w) {
				x=0;
				y++;
			}
			//std::cout << surface->pixels[i];
			if (((unsigned char *) surface->pixels)[i]==0) {
				*(ptr++) = 0;
				*(ptr++) = 0;
				*(ptr++) = 0;
				*(ptr++) = 0;
			} else {
				if (litx<0) {
					litx=x;
					lity=y;
				}
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
			}
		}
		if (litx<0) {
			lit = ImVec2(0.0f, 0.0f);
		} else {
			lit = ImVec2((float)litx/surface->w, (float)lity/surface->h);
		}
		glGenTextures(1, &gltex);
		if (gltex == 0) {
	    	std::cout << "Unable to init texture of size " << surface->w << "x" << surface->h << std::endl;
			SDL_FreeSurface(surface);
			SDL_FreeSurface(copy_surface);
	    	return gltex;
		}
		glBindTexture(GL_TEXTURE_2D, gltex);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, copy_surface->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		SDL_FreeSurface(surface);
		SDL_FreeSurface(copy_surface);
		return gltex;
    }

	void Font::init(std::string name, const void *data, int size, bool scandoubler) {
		gl_texture = 0;
		this->width = 0;
		this->height = 0;
		this->scandoubler = scandoubler;
		this->name = name;
		SDL_RWops *rw = SDL_RWFromConstMem(data, size);
		if (rw != NULL) {
			SDL_Surface *surface = IMG_LoadPNG_RW(rw);
			if (surface == NULL) {
				std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
			} else {
				width = surface->w/16;
				height = surface->h/16;
				gl_texture = surface_to_texture(surface);
			}
			SDL_RWclose(rw);
		}
	}

	Font::~Font() {
		if (gl_texture != 0) {
		    glDeleteTextures(1, &gl_texture);
			gl_texture = 0;
		}
	}
}
