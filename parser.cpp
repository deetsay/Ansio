#include "parser.h"

namespace parser {
	Parser::Parser(std::ifstream *ifile, merx::Merx *edit_area, int width, int height) {
		fg_color = 7;
		bg_color = 0;
		this->ifile = ifile;
		this->edit_area = edit_area;
		this->width = width;
		this->height = height;
	}

	Parser::~Parser() {
	}

	void Parser::write(int character) {
		if (load_done==true) {
			return;
		}
		int i=(y*width)+x;
		if (i>=(width*height)) {
			load_done = true;
			return;
		}
		if (character>=0 && character<0x100) {
			//std::cout << std::to_string(character) << std::endl;
			edit_area[i].character = character;
			edit_area[i].fg_color = fg_color;
			edit_area[i].bg_color = bg_color;
			x++;
			if (x>=width) {
				x=0;
				y++;
			}
		}
	}

	bool Parser::get() {
		if (load_done == true || !ifile->get(c)) {
			load_done = true;
			return false;
		}
		return true;
	}

	void Parser::parse_cmd() {
		int bold = 0;
		while (load_done==false) {
			bool processed = false;
			if (get() && c=='0') {
				if (get() && (c==';' || c=='m')) {
					fg_color=7;
					bg_color=0;
					processed = true;
				}
			} else if (c=='1') {
				if (get() && (c==';' || c=='m')) {
					bold = 8;
					fg_color = fg_color|bold;
					processed = true;
				}
			} else if (c=='3') {
				if (get() && c=='9') {
					if (get() && (c==';' || c=='m')) {
						fg_color=7;
						processed = true;
					}
				} else if (c>='0' && c<='7') {
					if (get() && (c==';' || c=='m')) {
						fg_color=(c&7)|bold;
						processed = true;
					}
				}
			} else if (c=='4') {
				if (get() && c=='9') {
					if (get() && (c==';' || c=='m')) {
						bg_color=0;
						processed = true;
					}
				} else if (c>='0' && c<='7') {
					if (get() && (c==';' || c=='m')) {
						bg_color=(c&7);
						processed = true;
					}
				}
			}
			if (processed == false) {
				while (c!='m' && c!=';' && get()) {
				}
			}
			if (c=='m') {
				return;
			}
		}
	}

	void Parser::parse_esc() {
		if (get() && c=='[') {
			parse_cmd();
		} else {
			write(27);
			write(c);
		}
	}

	void Parser::parse_ansi() {
		x = 0;
		y = 0;
		for (int i=0; i<(width*height); i++) {
			edit_area[i].character = ' ';
			edit_area[i].fg_color = 7;
			edit_area[i].bg_color = 0;
		}
		load_done = false;
		while (load_done==false) {
			if (get()) {
				if (c==27) {
					parse_esc();
				} else if (c==0x0a) {
					x=0;
					y++;
					int i=(y*width);
					if (i>=width*height) {
						return;
					}
				} else if (c==0x09) {
					write(' ');
					write(' ');
					write(' ');
					write(' ');
					write(' ');
					write(' ');
					write(' ');
					write(' ');
				} else {
					write(c);
				}
			}
		}
	}
}
