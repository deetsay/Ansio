/*
 * Mi Casa
 *
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

#include "resources/topaz500.c"
#include "resources/topaz1200.c"
#include "resources/potnoodle.c"
#include "resources/xen.c"

std::string *filename;

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
					if (bg_color==0 && bg_new==0 && a->edit_area[i].character==' ') {
						skipped_spaces++;
					} else {
						while (skipped_lines>0) {
							ofile.put(0xa);
							skipped_lines--;
						}
						while (skipped_spaces>0) {
							ofile.put(' ');
							skipped_spaces--;
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
			ofile.close();
		}

	} else if (res == NFD_ERROR) {
		std::cout << "Error: " << NFD_GetError() << std::endl;
	}
}

void draw_char(merx::Font *font, int character, ImVec4 fg, ImVec4 bg, const ImVec2 p, ImDrawList *draw_list) {
	draw_list->AddRectFilled(
		p,
		ImVec2(p.x+font->width, p.y+font->height),
		ImGui::ColorConvertFloat4ToU32(bg));
	float x = (float) (character&15) / 16.0f;
	float y = (float) (character>>4) / 16.0f;
	ImGui::Image(
		(void *)(intptr_t)font->gl_texture,
		ImVec2(font->width, font->height),
		ImVec2(x, y),
		ImVec2(x+0.0625, y+0.0625),
		fg,
		ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
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
    //style.WindowBorderSize = 0.0f;
    //style.WindowPadding = ImVec2(0.0f,0.0f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    //style.GrabRounding = 0.0f;
    //style.PopupRounding = 0.0f;
    //style.ScrollbarRounding = 0.0f;

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

	//merx::Font *topaz500font = new merx::Font(topaz500, sizeof(topaz500));
	merx::Font *topaz1200font = new merx::Font(topaz1200, sizeof(topaz1200));
	topaz1200font->width=8; topaz1200font->height=16;
	merx::Font *potnoodlefont = new merx::Font(potnoodle, sizeof(potnoodle));
	merx::Font *xenfont = new merx::Font(xen, sizeof(xen));

    ansio::Ansio *a = new ansio::Ansio(topaz1200font);

	SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));

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
					SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));
				}
				ImGui::Separator();
				if (a->font->height==8 && dblpal==false) {
					a->font->height=16;
					SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));
				}
				static int fontsel = 0;
				if (ImGui::RadioButton("Topaz", &fontsel, 0)) {
					a->font = topaz1200font;
					a->font->width=8; a->font->height=16;
					SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));
				}
				if (ImGui::RadioButton("P0T-NOoDLE", &fontsel, 1)) {
					a->font = potnoodlefont;
					a->font->width=8; a->font->height=16;
					SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));
				}
				if (ImGui::RadioButton("Xen", &fontsel, 2)) {
					a->font = xenfont;
					a->font->height=16;
					SDL_SetWindowSize(window, a->font->width*(a->width+16+8), a->font->height*(a->height+4));
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		ImGui::Columns(3, NULL, false);

		ImGui::SetColumnWidth(-1, a->font->width*16);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (int y=0, i=0; y<16; y++) {
			for (int x=0; x<16; x++, i++) {
				ImGui::PushID(i);
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, i, a->palette[a->current.fg_color], a->palette[a->current.bg_color], p, draw_list);
				if (ImGui::IsItemHovered()) {
					if (ImGui::IsMouseDown(0)) {
						a->current.character = i;
					}
				}
				if (x<15) {
					ImGui::SameLine();
				}
				ImGui::PopID();
			}
		}

		ImGui::NextColumn();
		ImGui::SetColumnWidth(-1, a->font->width*4);

		for (int pal=0; pal<16; pal++) {
			const ImVec2 p = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(p, ImVec2(p.x+32, p.y+16), ImGui::ColorConvertFloat4ToU32(a->palette[pal]));
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
				ImGui::PushID(i);
				//a->edit_area[i].render(a->current, a->collection, a->palette);
				merx::Merx *m = &a->edit_area[i];
				const ImVec2 p = ImGui::GetCursorScreenPos();
				draw_char(a->font, m->character, a->palette[m->fg_color], a->palette[m->bg_color], p, draw_list);
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
						ImVec4 bg = a->palette[a->current.bg_color];
						ImVec4 ehb_bg = ImVec4(bg.x, bg.y, bg.z, alpa);
						ImVec4 fg = a->palette[a->current.fg_color];
						ImVec4 ehb_fg = ImVec4(fg.x, fg.y, fg.z, alpa);
						draw_char(a->font, a->current.character, ehb_fg, ehb_bg, p, draw_list);
					}
				}



				if (x<a->width-1) {
					ImGui::SameLine();
				}
				ImGui::PopID();
			}
		}

		if (hover.character != -1) {
			std::string status = "x=" + std::to_string(hover.x) + " y=" + std::to_string(hover.y)
				+ " char=" + std::to_string(hover.character);
			//status << hover.x << " Y=" << hover.y;
			ImGui::Text(status.c_str());
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
