#include "parser.h"

namespace parser {

	Parser::Parser(std::ifstream *ifile, merx::Merx *edit_area, int width, int height) {
		this->ifile = ifile;
		this->edit_area = edit_area;
		this->width = width;
		this->height = height;
	}

	void Parser::reset() {
		fg_color = 7;
		bg_color = 0;
		x = 0;
		y = 0;
		for (int i=0; i<(width*height); i++) {
			edit_area[i].character = ' ';
			edit_area[i].fg_color = 7;
			edit_area[i].bg_color = 0;
		}
		was_d = false;
		load_done = false;
	}

	Parser::~Parser() {
		if (ifile != NULL) {
			ifile->close();
		}
	}

	bool Parser::get() {
		if (load_done == true || !ifile->get(c)) {
			load_done = true;
			return false;
		}
		got_chars++;
		return true;
	}

	void Parser::write(int character) {
		if (load_done==true) {
			return;
		}
		while (x>=width) {
			x-=width;
			y++;
		}
		while (y >= height) {
			for (int i=0; i<(width*(height-1)); i++) {
				edit_area[i].character = edit_area[i+width].character;
				edit_area[i].fg_color = edit_area[i+width].fg_color;
				edit_area[i].bg_color = edit_area[i+width].bg_color;
			}
			for (int i=width*(height-1); i<(width*height); i++) {
				edit_area[i].character = ' ';
				edit_area[i].fg_color = 7;
				edit_area[i].bg_color = 0;
			}
			y--;
		}
		int i=(y*width)+x;
		edit_area[i].character = (unsigned char) character;
		edit_area[i].fg_color = fg_color;
		edit_area[i].bg_color = bg_color;
		x++;
	}

	int Parser::parse_csi_num(int default_num, int invalid_num) {
		int csi_num = 0;
		int csi_start = csi_idx;
		for (;csi_idx<csi_buf.size(); csi_idx++) {
			char d = csi_buf[csi_idx];
			if (d>='0' && d<='9') {
				csi_num = csi_num * 10;
				csi_num += d&0x0f;
			} else if (d == ';') {
				if (csi_start==csi_idx) {
					csi_idx++;
					return default_num;
				}
				csi_done = (++csi_idx) >= csi_buf.size();
				return csi_num;
			} else {
				csi_done = (++csi_idx) >= csi_buf.size();
				return invalid_num;
			}
		}
		if (csi_start==csi_idx) {
			csi_done = (++csi_idx) >= csi_buf.size();
			return default_num;
		}
		csi_done = (++csi_idx) >= csi_buf.size();
		return csi_num;
	}

	// For CSI, or "Control Sequence Introducer" commands, the ESC [ is
	// followed by any number (including none) of
	// "parameter bytes" in the range 0x30–0x3F (ASCII 0–9:;<=>?),
	// then by any number of
	// "intermediate bytes" in the range 0x20–0x2F (ASCII space and !"#$%&'()*+,-./),
	// then finally by a single
	// "final byte" in the range 0x40–0x7E (ASCII @A–Z[\]^_`a–z{|}~).[5]:5.4
	void Parser::parse_csi() {
		csi_buf.clear();
		csi_idx = 0;
		csi_done = false;
		while (get() && c>=0x30 && c<=0x3f) {
			csi_buf.push_back(c);
		}
		if (c>=0x20 && c<=0x2f) {
			csi_buf.push_back(c);
			while (get() && c>=0x20 && c<=0x2f) {
				csi_buf.push_back(c);
			}
		}
		if (c>=0x40 && c<=0x7e) {
			if (c=='m') {
				int bold = 0;
				while (csi_done == false) {
					int sgr_num = parse_csi_num(0, -1);
					if (sgr_num == 0) {
						fg_color = 7;
						bg_color = 0;
					} else if (sgr_num == 1) {
						bold = 8;
						fg_color = fg_color|bold;
					} else if (sgr_num >= 30 && sgr_num <= 37) {
						fg_color = (sgr_num-30)+bold;
					} else if (sgr_num == 39) {
						fg_color = 7;
					} else if (sgr_num >= 40 && sgr_num <= 47) {
						bg_color = (sgr_num-40)+bold;
					} else if (sgr_num == 49) {
						bg_color = 0;
					}
				}
			}
			if (c=='A') {
				y -= parse_csi_num(1, 0);
				if (y < 0) {
					y = 0;
				}
			}
			if (c=='B') {
				y += parse_csi_num(1, 0);
			}
			if (c=='C') {
				x += parse_csi_num(1, 0);
			}
			if (c=='D') {
				x -= parse_csi_num(1, 0);
				if (x < 0) {
					x = 0;
				}
			}
			if (c=='H' || c=='f') {
				int xset = parse_csi_num(1, -1);
				int yset = parse_csi_num(1, -1);
				if (xset >= 1) {
					x = xset-1;
				}
				if (x > width - 1) {
					x = width - 1;
				}
				if (yset >= 1) {
					y = yset-1;
				}
				if (y > height - 1) {
					y = height - 1;
				}
			}
		}
	}

	int Parser::parse_ansi() {
		got_chars = 0;
		if (load_done==true) {
			return got_chars;
		}
		if (get()) {
			if (was_d && (c!=0x0a)) {
				write(0x0d);
				was_d = false;
			}
			if (c==27) {
				if (get() && c=='[') {
					parse_csi();
				} else {
					write(27);
					write(c);
				}
			} else if (c==0x0d) {
				was_d = true;
			} else if (c==0x0a) {
				x=0;
				y++;
				was_d = false;
			} else if (c==0x09) {
				write(' ');
				while (x&7!=0) {
					write(' ');
				}
			} else {
				write(c);
			}
		}
		if (was_d == true) {
			write(0x0d);
		}
		return got_chars;
	}
}
