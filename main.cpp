/*
 *	Ansio
 *
 *	TODO:
 *	- open file from command line
 *  - fix load & save
 *  - undo
 *  - palettes!
 *  - hide control characters etc with options
 */

// Based on Dear ImGui's standalone example application for SDL2 + OpenGL

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "nfd.h"

#include "ansio.h"
#include "parser.h"

#include "resources/topaz1200.c"
#include "resources/potnoodle.c"
#include "resources/xen.c"
#include "resources/lamers.c"

std::string *filename;
merx::Font topaz1200font;

void load(ansio::Ansio *a) {
	char *file = NULL;
	nfdresult_t res = NFD_OpenDialog(NULL, NULL, &file);
	if (res == NFD_OKAY) {
		if (filename != NULL) {
			delete filename;
			filename = NULL;
		}
		filename = new std::string(file);
		free(file);

		std::ifstream ifile;
		ifile.open(filename->c_str(), std::ios::in|std::ios::binary);
		if (ifile.is_open()) {
			parser::Parser *parsa = new parser::Parser(&ifile, a->edit_area, a->width, a->height);
			parsa->parse_ansi();
			delete parsa;
			ifile.close();
		}
	} else if (res == NFD_ERROR) {
		std::cout << "Error: " << NFD_GetError() << std::endl;
	}
}

void save(ansio::Ansio *a) {
	char *file = NULL;
	nfdresult_t res = NFD_SaveDialog(NULL, NULL, &file);
	if (res == NFD_OKAY) {
		if (filename != NULL) {
			delete filename;
			filename = NULL;
		}
		filename = new std::string(file);
		free(file);

		std::ofstream ofile;
		ofile.open(filename->c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
		if (ofile.is_open()) {

			int fg_color = 7;
			int bg_color = 0;
			int skipped_lines = 0;
			for (int y=0, i=0; y<a->height; y++) {
				int skipped_spaces = 0;
				for (int x=0; x<a->width; x++, i++) {
					int bg_new = a->edit_area[i].bg_color;
					if (bg_new==0 && a->edit_area[i].character==' ') {
						skipped_spaces++;
					} else {
						while (skipped_lines>0) {
							ofile.put(0xa);
							skipped_lines--;
						}
						if (skipped_spaces>0) {
							if (bg_color != 0) {
								ofile.put(27);
								ofile.put('[');
								ofile.put('4');
								ofile.put('0');
								ofile.put('m');
							}
							bg_color = 0;
							while (skipped_spaces>0) {
								ofile.put(' ');
								skipped_spaces--;
							}
						}
						int fg_new = a->edit_area[i].fg_color;
						if (fg_color != fg_new || bg_color != bg_new) {
							ofile.put(27);
							ofile.put('[');
							if (fg_color != fg_new) {
								if (fg_new>=8) {
									ofile.put('1');
									ofile.put(';');
								}
								ofile.put('3');
								ofile.put('0'+(fg_new & 0x7));
								if (bg_color != bg_new) {
									ofile.put(';');
								}
								fg_color = fg_new;
							}
							if (bg_color != bg_new) {
								ofile.put('4');
								ofile.put('0'+(bg_new & 0x7));
								bg_color = bg_new;
							}
							ofile.put('m');
						}
						ofile.put(a->edit_area[i].character);
					}
				}
				if (skipped_spaces == a->width) {
					skipped_lines++;
				} else {
					ofile.put(0xa);
				}
			}
			ofile.put(0xa);
			ofile.put(27);
			ofile.put('[');
			ofile.put('0');
			ofile.put('m');
			ofile.put(0xa);
			ofile.close();
		}

	} else if (res == NFD_ERROR) {
		std::cout << "Error: " << NFD_GetError() << std::endl;
	}
}

void draw_char(merx::Font *font, int character, ImVec4 fg, ImVec4 bg, const ImVec2 p) {

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(
		(void *)(intptr_t)font->gl_texture,
		p,
		ImVec2(p.x+font->width, p.y+font->height),
		font->lit,
		ImVec2(font->lit.x+0.0078125f, font->lit.y+0.0078125f),
		ImGui::GetColorU32(bg));
	float x = (float) (character&15) / 16.0f;
	float y = (float) (character>>4) / 16.0f;
	ImGui::Image(
		(void *)(intptr_t)font->gl_texture,
		ImVec2(font->width, font->height),
		ImVec2(x, y),
		ImVec2(x+0.0625f, y+0.0625f),
		fg,
		ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void set_window_size(SDL_Window *window, ansio::Ansio *a) {
	SDL_SetWindowSize(window,
		a->font->width*(a->width+16+8), a->font->height*(a->height+2)+32);
}

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Ansio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 8*(80+16+8), 27*16, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); //(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = NULL;

    //io.Fonts->AddFontFromMemoryCompressedTTF(potnoodle_compressed_data, potnoodle_compressed_size, 16.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(0.0f,0.0f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

	//merx::Font *topaz500font = new merx::Font(topaz500, sizeof(topaz500));
	topaz1200font.init(topaz1200, sizeof(topaz1200));
	topaz1200font.width=8;
	topaz1200font.height=16;
	merx::Font potnoodlefont;
	potnoodlefont.init(potnoodle, sizeof(potnoodle));
	merx::Font xenfont;
	xenfont.init(xen, sizeof(xen));
	merx::Font lamersfont;
	lamersfont.init(lamers, sizeof(lamers));

	merx::Palette ks13pal;
	ks13pal.size=4;
	ks13pal.color[0] = ImVec4(0.0f, 0.332f, 0.664f, 1.0f);
	ks13pal.color[1] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ks13pal.color[2] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ks13pal.color[3] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
	merx::Palette amipal;
	amipal.size=8;
	amipal.color[0] = ImVec4(0.664f, 0.664f, 0.664f, 1.0f);
	amipal.color[1] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	amipal.color[2] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	amipal.color[3] = ImVec4(0.3984f, 0.5313f, 0.73f, 1.0f);
	amipal.color[4] = ImVec4(0.9297f, 0.2656f, 0.2656f, 1.0f);
	amipal.color[5] = ImVec4(0.332f, 0.8633f, 0.332f, 1.0f);
	amipal.color[6] = ImVec4(0.0f, 0.2656f, 0.8633f, 1.0f);
	amipal.color[7] = ImVec4(0.9297f, 0.5977f, 0.0f, 1.0f);

	merx::Palette mwbpal;
	mwbpal.size=8;
	mwbpal.color[0] = ImVec4(0.582f, 0.582f, 0.582f, 1.0f);
	mwbpal.color[1] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	mwbpal.color[2] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	mwbpal.color[3] = ImVec4(0.23f, 0.4023f, 0.6328f, 1.0f);
	mwbpal.color[4] = ImVec4(0.48f, 0.48f, 0.48f, 1.0f);
	mwbpal.color[5] = ImVec4(0.6836f, 0.6836f, 0.6836f, 1.0f);
	mwbpal.color[6] = ImVec4(0.664f, 0.5625f, 0.4844f, 1.0f);
	mwbpal.color[7] = ImVec4(1.0f, 0.66f, 0.5898f, 1.0f);

	merx::Palette bterm;
	bterm.size=8;
	bterm.color[0] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	bterm.color[1] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	bterm.color[2] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	bterm.color[3] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	bterm.color[4] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	bterm.color[5] = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	bterm.color[6] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
	bterm.color[7] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	merx::Palette dterm;
	dterm.size=16;
	dterm.color[0] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	dterm.color[1] = ImVec4(0.73f, 0.0f, 0.0f, 1.0f);
	dterm.color[2] = ImVec4(0.0f, 0.73f, 0.0f, 1.0f);
	dterm.color[3] = ImVec4(0.73f, 0.73f, 0.0f, 1.0f);
	dterm.color[4] = ImVec4(0.0f, 0.0f, 0.73f, 1.0f);
	dterm.color[5] = ImVec4(0.73f, 0.0f, 0.73f, 1.0f);
	dterm.color[6] = ImVec4(0.0f, 0.73f, 0.73f, 1.0f);
	dterm.color[7] = ImVec4(0.73f, 0.73f, 0.73f, 1.0f);
	dterm.color[8] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
	dterm.color[9] = ImVec4(1.0f, 0.33f, 0.33f, 1.0f);
	dterm.color[10] = ImVec4(0.33f, 1.0f, 0.33f, 1.0f);
	dterm.color[11] = ImVec4(1.0f, 1.0f, 0.33f, 1.0f);
	dterm.color[12] = ImVec4(0.33f, 0.33f, 1.0f, 1.0f);
	dterm.color[13] = ImVec4(1.0f, 0.33f, 1.0f, 1.0f);
	dterm.color[14] = ImVec4(0.33f, 1.0f, 1.0f, 1.0f);
	dterm.color[15] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	merx::Palette vgapal;
	vgapal.size=16;
	vgapal.color[0] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	vgapal.color[1] = ImVec4(0.0f, 0.0f, 0.664f, 1.0f);
	vgapal.color[2] = ImVec4(0.0f, 0.664f, 0.0f, 1.0f);
	vgapal.color[3] = ImVec4(0.0f, 0.664f, 0.664f, 1.0f);
	vgapal.color[4] = ImVec4(0.664f, 0.0f, 0.0f, 1.0f);
	vgapal.color[5] = ImVec4(0.664f, 0.0f, 0.664f, 1.0f);
	vgapal.color[6] = ImVec4(0.664f, 0.332f, 0.0f, 1.0f);
	vgapal.color[7] = ImVec4(0.664f, 0.664f, 0.664f, 1.0f);
	vgapal.color[8] = ImVec4(0.332f, 0.332f, 0.332f, 1.0f);
	vgapal.color[9] = ImVec4(0.332f, 0.332f, 1.0f, 1.0f);
	vgapal.color[10] = ImVec4(0.332f, 1.0f, 0.332f, 1.0f);
	vgapal.color[11] = ImVec4(0.332f, 1.0f, 1.0f, 1.0f);
	vgapal.color[12] = ImVec4(1.0f, 0.332f, 0.332f, 1.0f);
	vgapal.color[13] = ImVec4(1.0f, 0.332f, 1.0f, 1.0f);
	vgapal.color[14] = ImVec4(1.0f, 1.0f, 0.332f, 1.0f);
	vgapal.color[15] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ansio::Ansio *a = new ansio::Ansio(&topaz1200font, &ks13pal);

	set_window_size(window, a);

	filename = NULL;
	bool done = false;

    // Main loop
    while (done == false) {
		int action = 0;
		bool showcursor = true;
		merx::Undo hover;
		hover.character = -1;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done = true;
	    	} else if (event.type == SDL_KEYDOWN) {
				action = event.key.keysym.sym;
				if (action == SDLK_ESCAPE) {
					done = true;
				} else if (event.key.keysym.mod == KMOD_RCTRL || event.key.keysym.mod == KMOD_LCTRL) {
					if (action == SDLK_q) {
						done = true;
					} else if (action == SDLK_o) {
						load(a);
					} else if (action == SDLK_s) {
						save(a);
					} else if (action == SDLK_f) {
						if ((SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0) {
							SDL_SetWindowFullscreen(window, 0);
						} else {
							SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
					}
				} else {
					if ((event.key.keysym.mod == KMOD_LSHIFT) || (event.key.keysym.mod == KMOD_RSHIFT)) {
						if (action >= 'a' && action <= 'z') {
							a->current.character = action - 32;
						} else if (action >= 0xe0 && action <= 0xff) {
							a->current.character = action - 32;
						} else if (action >= 0x20 && action < 0xe0) {
							a->current.character = action;
						}
					} else if (action >= 0x20 && action < 0xe0) {
						a->current.character = action;
					}
				}
			}
        }

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

		ImGui::Begin("Ansio Main", NULL,
	    	ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing
	    	|ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
	    	|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Menu")) {
				if (ImGui::MenuItem("Open", "CTRL+O")) {
					load(a);
				}
				if (ImGui::MenuItem("Save", "CTRL+S")) {
					save(a);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "CTRL+Q")) {
					done = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Font")) {
				static bool dblpal = false;
				ImGui::Checkbox("DblPAL", &dblpal);
				if (a->font->height==16 && dblpal==true) {
					a->font->height=8;
					set_window_size(window, a);
				}
				if (a->font->height==8 && dblpal==false) {
					a->font->height=16;
					set_window_size(window, a);
				}
				ImGui::Separator();
				static int fontsel = 0;
				if (ImGui::RadioButton("Topaz", &fontsel, 0)) {
					a->font = &topaz1200font;
					a->font->width=8; a->font->height=16;
					set_window_size(window, a);
				}
				if (ImGui::RadioButton("P0T-NOoDLE", &fontsel, 1)) {
					a->font = &potnoodlefont;
					a->font->width=8; a->font->height=16;
					set_window_size(window, a);
				}
				if (ImGui::RadioButton("Lamererz", &fontsel, 2)) {
					a->font = &lamersfont;
					set_window_size(window, a);
				}
				if (ImGui::RadioButton("Xen", &fontsel, 3)) {
					a->font = &xenfont;
					a->font->height=16;
					set_window_size(window, a);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Palette")) {
				static int palsel = 0;
				if (ImGui::RadioButton("WB1.3", &palsel, 0)) {
					a->palette = &ks13pal;
				}
				if (ImGui::RadioButton("WB2.05", &palsel, 1)) {
					a->palette = &amipal;
				}
				if (ImGui::RadioButton("MagicWB", &palsel, 2)) {
					a->palette = &mwbpal;
				}
				if (ImGui::RadioButton("Bold terminal", &palsel, 3)) {
					a->palette = &bterm;
				}
				if (ImGui::RadioButton("Dim terminal", &palsel, 4)) {
					a->palette = &dterm;
				}
				if (ImGui::RadioButton("Lamererz", &palsel, 5)) {
					a->palette = &vgapal;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		ImGui::Columns(3, NULL, false);

		ImGui::SetColumnWidth(-1, a->font->width*16);

		for (int y=0, i=0; y<16; y++) {
			for (int x=0; x<16; x++, i++) {
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, i, a->palette->get_color(a->current.fg_color), a->palette->get_color(a->current.bg_color), p);
				if (ImGui::IsItemHovered()) {
					if (ImGui::IsMouseDown(0)) {
						a->current.character = i;
					}
				}
				if (x<15) {
					ImGui::SameLine();
				}
			}
		}

		ImGui::NextColumn();
		ImGui::SetColumnWidth(-1, a->font->width*4);

		for (int pal=0; pal<16; pal++) {
			const ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::Image(
				(void *)(intptr_t)topaz1200font.gl_texture,
				ImVec2(32.0f, 16.0f),
				ImVec2(0.0f, 0.0f),
				ImVec2(0.0625f / topaz1200font.width, 0.0625f / topaz1200font.height),
				a->palette->get_color(pal),
				ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
			);
			ImGui::SetCursorScreenPos(p);
			ImGui::Text("    ");
			if (ImGui::IsItemHovered()) {
				if (ImGui::IsMouseDown(0)) {
					a->current.fg_color = pal;
				} else if (ImGui::IsMouseDown(1)) {
					a->current.bg_color = pal;
				}
			}
		}

		ImGui::NextColumn();

		for (int y=0, i=0; y<a->height; y++) {
			for (int x=0; x<a->width; x++, i++) {
				merx::Merx *m = &a->edit_area[i];
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, m->character, a->palette->get_color(m->fg_color), a->palette->get_color(m->bg_color), p);
				if (ImGui::IsItemHovered()) {
					showcursor = false;
					hover.x = x;
					hover.y = y;
					hover.character = m->character;
					hover.fg_color = m->fg_color;
					hover.bg_color = m->bg_color;
					if (ImGui::IsMouseDown(0)) {
						showcursor = true;
						*m=a->current;
					} else if (ImGui::IsMouseDown(1)) {
						showcursor = true;
						*m=a->defaultMerx;
					} else {
						ImGui::SetCursorScreenPos(p);
						float alpa = (float) (sin((double)SDL_GetTicks()/128)*0.45f)+0.5f;
						ImVec4 fg = a->palette->get_color(a->current.fg_color);
						ImVec4 ehb_fg = ImVec4(fg.x, fg.y, fg.z, alpa);
						ImVec4 bg = a->palette->get_color(a->current.bg_color);
						ImVec4 ehb_bg = ImVec4(bg.x, bg.y, bg.z, alpa);
						draw_char(a->font, a->current.character, ehb_fg, ehb_bg, p);
					}
				}
				if (x<a->width-1) {
					ImGui::SameLine();
				}
			}
		}

		if (hover.character != -1) {
			//char *status = new char[80];
			//sprintf(status, );
			ImGui::Text("x=%d y=%d char=%d", hover.x, hover.y, hover.character);
			//delete [] status;
		}

		ImGui::PopStyleVar();

		ImGui::End();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);


		if (showcursor == true) {
			SDL_ShowCursor(SDL_ENABLE);
		} else {
			SDL_ShowCursor(SDL_DISABLE);
		}

		//glClearColor(255, 255, 255, 255);
		//glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
    }

	if (filename != NULL) {
		delete filename;
		filename = NULL;
	}
    delete a;
	a = NULL;

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
