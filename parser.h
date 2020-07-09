#include <iostream>
#include <fstream>
#include <vector>

#include "merx.h"

namespace parser {
	class Parser {
	private:
		std::vector<char> csi_buf;
		int csi_idx;
		bool csi_done;

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

		bool get();
		void write(int character);

		int parse_csi_num(int default_num, int invalid_num);
		void parse_csi();

	public:
		Parser(std::ifstream *ifile, merx::Merx *edit_area, int width, int height);
		virtual ~Parser();

		void parse_ansi();
	};
}
