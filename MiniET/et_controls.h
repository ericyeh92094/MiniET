#pragma once

#include "hpdf.h"

using namespace std;

class et_controls {
public:
	// attributes
	HPDF_INT	P;
	HPDF_INT	D,      // 0 to 7
		T,      // 16 .. 256
		F,      // 'A' .. 'Z', 255 = user given
		W,      // 1..255
		Z,      // 1..255
		R,      // 1:(normal) 2:(reversed)
		VorH,   // 'V' or 'H'
		CorE,   // 'C' or 'E'
		U,      // 1:(stop underline) 2:(underline)
				// 3:(stop upperline) 4:(upperline)
				// bit0: underline bit1: upperline
		G,      // 1:(not connect frame) 2:(connet frame)
		GT,     // 1: Top, 2: Middle, 3: Bottom
		B,      // 0 to 6 (0 default)
		J,      // 0 - Left, 1 - Middle, 2 - Right, 3 - (reserved)
		O,		// 0 - 15 color level
		Q,		// # of spaces before line
		TX, TY; // 16 - 4096, 0 = not set
	HPDF_INT
		L,      // -255..255
		X;      // -255..255

	HPDF_INT S;      // 0 or non 0

	enum JTYPE { J_LEFT, J_MIDDLE, J_RIGHT };

	HPDF_INT	LC; // 0 - 255, -1 = not set

	HPDF_BOOL	HasPosition;
	HPDF_INT	Px, Py, Rx, Ry;

	HPDF_REAL	text_color_r, text_color_g, text_color_b;  // Text color
	HPDF_REAL	table_color_r, table_color_g, table_color_b; // Table Color
	HPDF_REAL	line_color_r, line_color_g, line_color_b;  // Under Line, ~[H]
	HPDF_REAL	bg_color_r, bg_color_g, bg_color_b; // Background Color, ~R

	string		c_ext_font_name, e_ext_font_name;

	inline void use_eng_font(string font_name)
	{
		e_ext_font_name = font_name;
	}

	inline void use_cjk_font(string font_name)
	{
		c_ext_font_name = font_name;
	}
	inline HPDF_INT use_font(wstring font_letter)
	{
		return (HPDF_INT)(font_letter[0] - L'A');
	}
	void init(int p_code);
};

