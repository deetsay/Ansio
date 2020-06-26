#include "merx.h"
#include <iostream>

namespace merx {

	GLuint Font::surface_to_texture(SDL_Surface *surface) {

		GLuint gltex = 0;
		SDL_Surface *copy_surface = SDL_CreateRGBSurfaceWithFormat(
			0, surface->w, surface->h, 32, SDL_PIXELFORMAT_ABGR8888);
		if (copy_surface == NULL) {
			SDL_FreeSurface(surface);
			return gltex;
		}

		unsigned char *ptr = (unsigned char *) copy_surface->pixels;
		for (int i=0; i<surface->w*surface->h; i++) {
			//std::cout << surface->pixels[i];
			if (((unsigned char *) surface->pixels)[i]==0) {
				*(ptr++) = 0;
				*(ptr++) = 0;
				*(ptr++) = 0;
				*(ptr++) = 0;
			} else {
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
				*(ptr++) = 0xff;
			}
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		SDL_FreeSurface(surface);
		SDL_FreeSurface(copy_surface);
		return gltex;
    }

	Font::Font(const void *data, int size) {
		gl_texture = 0;
		this->width = 0;
		this->height = 0;
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
