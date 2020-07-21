/*
 *	Ansio
 *
 *	TODO:
 *	- open file from command line
 *  - undo
 *  - bold
 *  - italic
 *  - underscore
 *  - hide/mark control characters etc with options
 */

// Based on Dear ImGui's standalone example application for SDL2 + OpenGL

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <math.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
//#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "nfd.h"

#include "ansio.h"
#include "parser.h"

#include "resources/topaz500.c"
#include "resources/topaz1200.c"
#include "resources/potnoodle.c"
#include "resources/xen.c"
#include "resources/lamers.c"
#include "resources/petscii-ucase.c"
#include "resources/petscii-lcase.c"

std::string *filename;
std::ifstream ifile;
parser::Parser *parsa;

void load(ansio::Ansio *a) {
	parsa = NULL;
	char *file = NULL;
	nfdresult_t res = NFD_OpenDialog(NULL, NULL, &file);
	if (res == NFD_OKAY) {
		if (filename != NULL) {
			delete filename;
			filename = NULL;
		}
		filename = new std::string(file);
		free(file);

		ifile.open(filename->c_str(), std::ios::in|std::ios::binary);
		if (ifile.is_open()) {
			parsa = new parser::Parser(&ifile, a->edit_area, a->width, a->height);
			parsa->reset();
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

void draw_char(merx::Font *font, int character, ImVec4 fg, ImVec4 bg,
	const ImVec2 p, bool bold, float zoom) {

	if (ImGui::IsRectVisible(ImVec2(font->width*zoom, font->height*zoom))==false) {
		ImGui::Dummy(ImVec2(font->width*zoom, font->height*zoom));
		return;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(
		(void *)(intptr_t)font->gl_texture,
		p,
		ImVec2(p.x+(font->width*zoom), p.y+(font->height*zoom)),
		font->lit,
		ImVec2(font->lit.x+0.0078125f, font->lit.y+0.0078125f),
		ImGui::GetColorU32(bg));
	float x = (float) (character&15) / 16.0f;
	float y = (float) (character>>4) / 16.0f;
	if (bold) {
		draw_list->AddImage(
			(void *)(intptr_t)font->gl_texture,
			ImVec2(p.x+1, p.y),
			ImVec2(p.x+1+(font->width*zoom), p.y+(font->height*zoom)),
			ImVec2(x, y),
			ImVec2(x+0.0625f, y+0.0625f),
			ImGui::GetColorU32(fg));
	}
	ImGui::Image(
		(void *)(intptr_t)font->gl_texture,
		ImVec2(font->width*zoom, font->height*zoom),
		ImVec2(x, y),
		ImVec2(x+0.0625f, y+0.0625f),
		fg,
		ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
	);
}

/*void set_window_size(SDL_Window *window, ansio::Ansio *a) {
	SDL_SetWindowSize(window,
		a->font->width*(a->width+16+8), a->font->height*(a->height+2)+32);
}*/

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
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
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
    //ImGui::StyleColorsLight();
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

	std::vector<merx::Font*> fonts;

	merx::Font topaz500font;
	topaz500font.init("Topaz", topaz500, sizeof(topaz500), true);
	fonts.push_back(&topaz500font);
	merx::Font topaz1200font;
	topaz1200font.init("Topaz New", topaz1200, sizeof(topaz1200), true);
	topaz1200font.width=8;
	topaz1200font.height=16;
	fonts.push_back(&topaz1200font);
	merx::Font potnoodlefont;
	potnoodlefont.init("P0T-NOoDLE", potnoodle, sizeof(potnoodle), true);
	fonts.push_back(&potnoodlefont);
	merx::Font lamersfont;
	lamersfont.init("EGA/VGA", lamers, sizeof(lamers), false);
	fonts.push_back(&lamersfont);
	merx::Font xenfont;
	xenfont.init("Xen", xen, sizeof(xen), true);
	fonts.push_back(&xenfont);
	merx::Font petscii_ucfont;
	petscii_ucfont.init("PETSCII Uppercase", petscii_ucase, sizeof(petscii_ucase), false);
	fonts.push_back(&petscii_ucfont);
	merx::Font petscii_lcfont;
	petscii_lcfont.init("PETSCII Lowercase", petscii_lcase, sizeof(petscii_lcase), false);
	fonts.push_back(&petscii_lcfont);

	std::vector<merx::Palette*> palettes;

	merx::Palette ks13pal;
	ks13pal.name = "Workbench";
	ks13pal.push_back(0x0055aa);
	ks13pal.push_back(0xffffff);
	ks13pal.push_back(0x000000);
	ks13pal.push_back(0xff8800);
	palettes.push_back(&ks13pal);

	merx::Palette amipal;
	amipal.name = "Workbench New";
	amipal.push_back(0xaaaaaa);
	amipal.push_back(0x000000);
	amipal.push_back(0xffffff);
	amipal.push_back(0x6688bb);
	amipal.push_back(0xee4444);
	amipal.push_back(0x55dd55);
	amipal.push_back(0x0044dd);
	amipal.push_back(0xee9900);
	palettes.push_back(&amipal);

	merx::Palette mwbpal;
	mwbpal.name = "MagicWB";
	mwbpal.push_back(0x959595);
	mwbpal.push_back(0x000000);
	mwbpal.push_back(0xffffff);
	mwbpal.push_back(0x3b67a2);
	mwbpal.push_back(0x7b7b7b);
	mwbpal.push_back(0xafafaf);
	mwbpal.push_back(0xaa907c);
	mwbpal.push_back(0xffa997);
	palettes.push_back(&mwbpal);

	merx::Palette bterm;
	bterm.name = "Terminal";
	bterm.push_back(0x000000);
	bterm.push_back(0xff0000);
	bterm.push_back(0x00ff00);
	bterm.push_back(0xffff00);
	bterm.push_back(0x0000ff);
	bterm.push_back(0xff00ff);
	bterm.push_back(0x00ffff);
	bterm.push_back(0xffffff);
	palettes.push_back(&bterm);

	merx::Palette dterm;
	dterm.name = "Terminal Dim";
	dterm.push_back(0x000000);
	dterm.push_back(0xbb0000);
	dterm.push_back(0x00bb00);
	dterm.push_back(0xbbbb00);
	dterm.push_back(0x0000bb);
	dterm.push_back(0xbb00bb);
	dterm.push_back(0x00bbbb);
	dterm.push_back(0xbbbbbb);
	dterm.push_back(0x555555);
	dterm.push_back(0xff5555);
	dterm.push_back(0x55ff55);
	dterm.push_back(0xffff55);
	dterm.push_back(0x5555ff);
	dterm.push_back(0xff55ff);
	dterm.push_back(0x55ffff);
	dterm.push_back(0xffffff);
	palettes.push_back(&dterm);

	merx::Palette vgapal;
	vgapal.name = "VGA";
	vgapal.push_back(0x000000);
	vgapal.push_back(0x0000aa);
	vgapal.push_back(0x00aa00);
	vgapal.push_back(0x00aaaa);
	vgapal.push_back(0xaa0000);
	vgapal.push_back(0xaa00aa);
	vgapal.push_back(0xaa5500);
	vgapal.push_back(0xaaaaaa);
	vgapal.push_back(0x555555);
	vgapal.push_back(0x5555ff);
	vgapal.push_back(0x55ff55);
	vgapal.push_back(0x55ffff);
	vgapal.push_back(0xff5555);
	vgapal.push_back(0xff55ff);
	vgapal.push_back(0xffff55);
	vgapal.push_back(0xffffff);
	palettes.push_back(&vgapal);

	merx::Palette pepto;
	pepto.name = "Pepto";
	pepto.push_back(0x000000);
	pepto.push_back(0xFFFFFF);
	pepto.push_back(0x68372B);
	pepto.push_back(0x70A4B2);
	pepto.push_back(0x6F3D86);
	pepto.push_back(0x588D43);
	pepto.push_back(0x352879);
	pepto.push_back(0xB8C76F);
	pepto.push_back(0x6F4F25);
	pepto.push_back(0x433900);
	pepto.push_back(0x9A6759);
	pepto.push_back(0x444444);
	pepto.push_back(0x6C6C6C);
	pepto.push_back(0x9AD284);
	pepto.push_back(0x6C5EB5);
	pepto.push_back(0x959595);
	palettes.push_back(&pepto);

	merx::Palette colodore;
	colodore.name = "Colodore";
	colodore.push_back(0x000000);
	colodore.push_back(0xffffff);
	colodore.push_back(0x813338);
	colodore.push_back(0x75cec8);
	colodore.push_back(0x8e3c97);
	colodore.push_back(0x56ac4d);
	colodore.push_back(0x2e2c9b);
	colodore.push_back(0xedf171);
	colodore.push_back(0x8e5029);
	colodore.push_back(0x553800);
	colodore.push_back(0xc46c71);
	colodore.push_back(0x4a4a4a);
	colodore.push_back(0x7b7b7b);
	colodore.push_back(0xa9ff9f);
	colodore.push_back(0x706deb);
	colodore.push_back(0xb2b2b2);
	palettes.push_back(&colodore);

    ansio::Ansio *a = new ansio::Ansio(&topaz1200font, &bterm, 80, 25);

	//set_window_size(window, a);

	filename = NULL;
	bool done = false;

    // Main loop
    while (done == false) {
		int action = 0;
		bool showcursor = true;
		bool make_copy = false;
		merx::Undo hover;
		hover.character = -1;

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
					} else if (action == SDLK_c) {
						make_copy = true;
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
	    	|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar
			|ImGuiWindowFlags_NoScrollbar);

		char menu_action=0;
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Menu")) {
				if (ImGui::MenuItem("New", "CTRL-N")) {
					menu_action='n';
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Load", "CTRL-L")) {
					menu_action='l';
				}
				if (ImGui::MenuItem("Save", "CTRL-S")) {
					save(a);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "CTRL-Q")) {
					done = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Copy", "CTRL-C")) {
					make_copy = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Font")) {
				static bool dblpal = false;
				if (a->font->scandoubler == true) {
					ImGui::Checkbox("Scandoubler", &dblpal);
					if (a->font->height==16 && dblpal==true) {
						a->font->height=8;
					}
					if (a->font->height==8 && dblpal==false) {
						a->font->height=16;
					}
					ImGui::Separator();
				}
				static int fontsel = 0;
				int i=0;
				for (merx::Font *font : fonts) {
					if (ImGui::RadioButton(font->name.c_str(), &fontsel, i++)) {
						a->font = font;
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Palette")) {
				static int palsel = 3;
				int i=0;
				for (merx::Palette *pal : palettes) {
					if (ImGui::RadioButton(pal->name.c_str(), &palsel, i++)) {
						a->palette = pal;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (menu_action=='n') {
			ImGui::OpenPopup("New");
		}
		if (menu_action=='l') {
			load(a);
			ImGui::OpenPopup("Load");
		}
		//bool newopen = false;
		if (ImGui::BeginPopupModal("New")) {
			ImGui::Text("Foo");
			static int in_width = 80;
			static int in_height = 25;
			ImGui::InputInt("Width", &in_width);
			ImGui::InputInt("Height", &in_height);
			if (ImGui::Button("OK", ImVec2(120, 40))) {
				delete a;
				a = new ansio::Ansio(&topaz1200font, &bterm, in_width, in_height);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 40))) {
				ImGui::CloseCurrentPopup();
			}
		   ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Load")) {
			if (parsa != NULL) {
				int target = 20;
				int last_num = 0;
				do {
					last_num = parsa->parse_ansi();
					target -= last_num;
				} while (target >= 0 && last_num > 0);
				if (last_num == 0) {
					delete parsa;
					parsa = NULL;
				}
			}
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			for (int y=0, i=0; y<a->height; y++) {
				for (int x=0; x<a->width; x++, i++) {
					merx::Merx *m = &a->edit_area[i];
					const ImVec2 p = ImGui::GetCursorScreenPos();
					draw_char(a->font, m->character, a->palette->get_color(m->fg_color), a->palette->get_color(m->bg_color), p,
						a->palette->is_bold(m->fg_color), 1.0f);
					if (x<a->width-1) {
						ImGui::SameLine();
					}
				}
			}
			ImGui::PopStyleVar();

			if (parsa != NULL) {
				if (ImGui::Button("Stop", ImVec2(120, 40))) {
					delete parsa;
					parsa = NULL;
				}
			} else {
				if (ImGui::Button("Close", ImVec2(120, 40))) {
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Columns(2, NULL, false);

		ImGui::SetColumnWidth(-1, (a->font->width+1)*16);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));

		for (int y=0, i=0; y<16; y++) {
			for (int x=0; x<16; x++, i++) {
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, i, a->palette->get_color(a->current.fg_color), a->palette->get_color(a->current.bg_color), p,
					a->palette->is_bold(a->current.fg_color), 1.0f);
				if (ImGui::IsItemHovered()) {
					if (ImGui::IsMouseDown(0)) {
						a->current.character = i;
					}
					if (make_copy == true) {
						a->current.character = i;
					}
				}
				if (x<15) {
					ImGui::SameLine();
				}
			}
		}

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, a->palette->get_color(a->current.bg_color));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, a->palette->get_color(a->current.bg_color));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		int x=0;
		for (int pal=0; pal<a->palette->color.size(); pal++) {
			const ImVec2 p = ImGui::GetCursorScreenPos();
			if (pal==a->current.fg_color) {
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Button, a->palette->get_color(a->current.bg_color));
			}
			ImGui::ImageButton(
				(void *)(intptr_t)topaz1200font.gl_texture,
				ImVec2(24.0f, 24.0f),
				topaz1200font.lit,
				ImVec2(topaz1200font.lit.x+0.0078125f, topaz1200font.lit.y+0.0078125f),
				3,
				pal==a->current.fg_color ? a->palette->get_color(a->current.bg_color) :
					ImVec4(0.0f, 0.0f, 0.0f, 0.1f),
				a->palette->get_color(pal)
			);
			if (pal==a->current.fg_color) {
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			}
			//ImGui::SetCursorScreenPos(p);
			//ImGui::Text("    ");
			if (ImGui::IsItemHovered()) {
				if (ImGui::IsMouseDown(0)) {
					a->current.fg_color = pal;
				} else if (ImGui::IsMouseDown(1)) {
					a->current.bg_color = pal;
				}
			}
			x++;
			x = x & 3;
			if (x != 0) {
				ImGui::SameLine();
			}
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		float da_w = a->width*a->font->width*a->zoom;
		float da_h = a->height*a->font->height*a->zoom;
		ImGui::SetNextWindowContentSize(ImVec2(da_w, da_h));

		float dw_w = ImGui::GetWindowContentRegionMax().x-((a->font->width+1)*16)-8;
		float dw_h = ImGui::GetWindowContentRegionMax().y-34;

		float dwpad_x = 0.0f;
		float dwpad_y = 0.0f;

		if (da_w < dw_w) {
			dwpad_x = (dw_w-da_w)/2;
		}
		if (da_h < dw_h) {
			dwpad_y = (dw_h-da_h)/2;
		}

		const ImVec2 dw_p = ImGui::GetCursorScreenPos();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(dwpad_x, dwpad_y));

		ImGui::BeginChild("DrawScrollRegion",
			ImVec2(dw_w, dw_h),
			true,
			ImGuiWindowFlags_AlwaysHorizontalScrollbar
			| ImGuiWindowFlags_AlwaysVerticalScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse);

		//ImGui::SetCursorScreenPos(ImVec2(dw_p.x+dwpad_x, dw_p.y+dwpad_y));

		for (int y=0, i=0; y<a->height; y++) {
			for (int x=0; x<a->width; x++, i++) {
				merx::Merx *m = &a->edit_area[i];
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, m->character, a->palette->get_color(m->fg_color), a->palette->get_color(m->bg_color), p,
					a->palette->is_bold(m->fg_color), a->zoom);
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
						draw_char(a->font, a->current.character, ehb_fg, ehb_bg, p,
							a->palette->is_bold(a->current.fg_color), a->zoom);
					}
					if (make_copy == true) {
						a->current.character = m->character;
						a->current.fg_color = m->fg_color;
						a->current.bg_color = m->bg_color;
					}
				}
				if (x<a->width-1) {
					ImGui::SameLine();
				}
			}
		}

		if (ImGui::IsWindowHovered()) {
			ImGuiIO& io = ImGui::GetIO();
			float wheel = io.MouseWheel;
			if (wheel != 0.0f) {
				a->zoom = a->zoom + (wheel / 8);
				if (a->zoom < 1.0f) {
					a->zoom = 1.0f;
				}
				float new_w = a->width*a->font->width*a->zoom;
				float new_h = a->height*a->font->height*a->zoom;
				float dq_x;
				float dq_y;
				/*if (wheel > 0.0f) {
					dq_x = (1.2f*(io.MousePos.x-dw_p.x)/dw_w)-0.1f;
					dq_y = (1.2f*(io.MousePos.y-dw_p.y)/dw_h)-0.1f;
				} else {*/
					dq_x = (io.MousePos.x-dw_p.x)/dw_w;
					dq_y = (io.MousePos.y-dw_p.y)/dw_h;
				//}
				ImGui::SetScrollX(ImGui::GetScrollX()+((new_w-da_w)*dq_x));
				ImGui::SetScrollY(ImGui::GetScrollY()+((new_h-da_h)*dq_y));
			}
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();

		if (hover.character != -1) {
			ImGui::Text("x=%d y=%d char=%d", hover.x, hover.y, hover.character);
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
