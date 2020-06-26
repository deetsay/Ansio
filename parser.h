#include <iostream>
#include <fstream>

#include "merx.h"

namespace parser {
	class Parser {
	private:
		char c;
		bool load_done;
		int fg_color;
		int bg_color;
		int width;
		int height;
		merx::Merx *edit_area;
		int x;
		int y;
		std::ifstream *ifile;

		void write(int character);

		bool get();
		void parse_cmd();
		void parse_esc();

	public:
		Parser(std::ifstream *ifile, merx::Merx *edit_area, int width, int height);
		virtual ~Parser();

		void parse_ansi();
	};
}
