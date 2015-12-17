#include <math.h>
#include "et_hpdcdoc.h"
#include "include\utf8.h"

HPDF_REAL hpdf_doc::def_f_margin_top = PDF_DEFAULT_MARGIN_TOP;
HPDF_REAL hpdf_doc::def_f_margin_left = PDF_DEFAULT_MARGIN_LEFT;
HPDF_REAL hpdf_doc::def_f_margin_bottom = PDF_DEFAULT_MARGIN_BOTTOM;
HPDF_REAL hpdf_doc::def_f_margin_right = PDF_DEFAULT_MARGIN_RIGHT;
HPDF_REAL hpdf_doc::def_f_width = PDF_DEFAULT_WIDTH;
HPDF_REAL hpdf_doc::def_f_length = PDF_DEFAULT_LENGTH;
map< std::string, string> hpdf_doc::external_fonts;
string hpdf_doc::font_path_parent;


void hpdf_doc::set_paper_margins(HPDF_REAL width, HPDF_REAL length, HPDF_REAL top, HPDF_REAL left, HPDF_REAL bottom, HPDF_REAL right)
{
	def_f_width = width;
	def_f_length = length;
	def_f_margin_top = top;
	def_f_margin_left = left;
	def_f_margin_bottom = bottom;
	def_f_margin_right = right;
}

void hpdf_doc::set_fontpath_parent(string font_parent)
{
	font_path_parent = font_parent;
}

void hpdf_doc::set_paper_margin(wchar_t margin_indicator, HPDF_INT32 n_margin) // MMTEXT
{
	// margins
	switch (margin_indicator)
	{
	case L'l':
	case L'L':
		f_margin_left = (double)n_margin / 1000.0; // D2PTX(n_margin);
		break;
	case L't':
	case L'T':
		f_margin_top = (double)n_margin / 1000.0; //D2PTY(n_margin);
		break;
	case L'r':
	case L'R':
		f_margin_bottom = (double)n_margin / 1000.0; //D2PTY(n_margin);
		break;
	case L'b':
	case L'B':
		f_margin_right = (double)n_margin / 1000.0; //D2PTX(n_margin);
		break;
	}

	f_margin_x = INCH2PT(f_margin_left);
	f_margin_y = INCH2PT(f_margin_top);

	if (f_xpos < f_margin_x)
		f_xpos = f_margin_x;
	if (f_ypos > f_length - f_margin_y)
		f_ypos = f_length - f_margin_y;

}

void hpdf_doc::error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	/* throw exception when an error has occured */
	printf("ERROR: error_no=%04X, detail_no=%d\n", (unsigned int)error_no,
		(int)detail_no);
	throw std::exception();
}

hpdf_doc::hpdf_doc(const char* filepath) 
{
	h_pdf = HPDF_New(error_handler, NULL); 

	HPDF_UseCNTFonts(h_pdf);
	HPDF_UseCNTEncodings(h_pdf); // set CHT fonts and encoding

	HPDF_UseUTFEncodings(h_pdf);

	strcpy(filename, filepath);

	f_page_width = def_f_width;
	f_page_length = def_f_length;
	f_margin_top = def_f_margin_top;
	f_margin_left = def_f_margin_left;
	f_margin_bottom = def_f_margin_bottom;
	f_margin_right = def_f_margin_right;

	h_direction = HPDF_PAGE_PORTRAIT;
}

hpdf_doc::~hpdf_doc()
{ 
	HPDF_Free(h_pdf);

}

void hpdf_doc::add_external_font(string font_name, string font_path)
{
	external_fonts[font_name] = font_path;
}

void hpdf_doc::init_font_table()
{
	wstring font_str(L"A");
	const char *font_name;
	string font_full_path = font_path_parent + PDF_STD_UNICODE;

	font_name = HPDF_LoadTTFontFromFile(h_pdf, font_full_path.c_str(), HPDF_TRUE);

	HPDF_Font font = HPDF_GetFont(h_pdf, font_name, PDF_STD_ENCODE);

	for (int i = 0; i < 26; i++)
	{
		font_str[0] = L'A' + i;
		mapped_font *fm = new mapped_font(font_name, PDF_STD_ENCODE);
		fm->font_handle = font;
		mapped_font_lookup_table[font_str] = fm;
	}

	font_full_path = font_path_parent + PDF_STD_KAI;
	font_name = HPDF_LoadTTFontFromFile(h_pdf, font_full_path.c_str(), HPDF_TRUE);
	font = HPDF_GetFont(h_pdf, font_name, PDF_STD_ENCODE);
	
	mapped_font *kai_fm = new mapped_font(font_name, PDF_STD_ENCODE);
	kai_fm->font_handle = font;
	mapped_font_lookup_table[L"K"] = kai_fm;

	for (map<string, string>::iterator j = external_fonts.begin(); j != external_fonts.end(); ++j)
	{
		wstring font_letter = StringToWstring(j->first);
		string font_path = j->second;

		font_full_path = font_path_parent + font_path;
		font_name = HPDF_LoadTTFontFromFile(h_pdf, font_full_path.c_str(), HPDF_TRUE);
		font = HPDF_GetFont(h_pdf, font_name, PDF_STD_ENCODE);

		mapped_font *fm = new mapped_font(font_name, PDF_STD_ENCODE);
		fm->font_handle = font;
		mapped_font_lookup_table[font_letter] = fm;
	}
}

void hpdf_doc::begin_doc_and_page()
{
	init_font_table();

	init(300); //default P300
	new_page(); // the first page added

	h_current_font = HPDF_GetFont(h_pdf, PDF_STD_CHT, PDF_ENCODE_CHT_H);

	set_font_handle(h_current_page, h_current_font);

}

void hpdf_doc::end_doc()
{
	end_page();
	HPDF_AttachFile(h_pdf, source_filename);
	HPDF_SaveToFile(h_pdf, (const char*)filename);
}


void hpdf_doc::get_logical_DPI(int p_code, int D, HPDF_INT &log_X, HPDF_INT &log_Y, int x_ratio, int y_ratio)
{
	if (p_code < 100)
	{
		log_Y = p_code == 9 ? 180 : 300;
		if (p_code == 9)
		{
			switch (D)
			{
			case 0:  log_X = 180;
				break;
			case 1: log_X = 120;
				break;
			case 2: log_X = 90;
				break;
			case 3: log_X = 60;
				break;
			case 4: log_X = 360;
				break;
			default:
				log_X = 180;
			}
		}
		else
			log_X = 300;
	}
	else
	{
		log_X = log_Y = p_code;
	}

	log_X = (HPDF_INT)((unsigned long)log_X * 100UL / x_ratio);
	log_Y = (HPDF_INT)((unsigned long)log_Y * 100UL / y_ratio);
}

void hpdf_doc::init(int p_code)
{
	P = p_code;
	if (p_code < 100)
	{
		W = Z = 1;
		if (p_code == 9 || p_code == 0)
		{
			L = 6;
			if (p_code == 0)
				Rx = Ry = 0;
		}
		else
		{
			L = 2;
		}
		X = 12;
	}
	else if (p_code == 300)
	{
		W = Z = 1; // 司法院要求改成 W1Z1
		L = 2;
		X = 12;
	}
	else
	{
		W = Z = 1;
		L = 6;
		X = 12;
	}

	T = 24;

	F = 0; // FA
	c_ext_font_name = e_ext_font_name = "";

	R = 1;
	VorH = 'H';
	CorE = 'E';
	U = 0;
	G = 2;
	GT = 1;
	B = 0;
	D = 0;
	Q = 0;

	if (Rx == 0 || Ry == 0)
		Rx = Ry = 100;

	text_color_r = text_color_g = text_color_b = 0.0;  // Text color
	table_color_r = table_color_g = table_color_b = 0.0; // Table Color
	bg_color_r = bg_color_g = bg_color_b = 15;
	line_color_r = line_color_g = line_color_b = 0;  // Under Line, ~[H]

	// default justification - left
	J = J_LEFT;

	TX = TY = 0;
	LC = -1;

	S = 0; // no bold

	get_logical_DPI(p_code, D, n_log_X, n_log_Y, Rx, Ry); // Calc the logical DPI

}

void hpdf_doc::new_page()
{
	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;

	h_current_page = HPDF_AddPage(h_pdf);

	HPDF_Page_SetSize(h_current_page, HPDF_PAGE_SIZE_A4, h_direction);

	f_width = INCH2PT(f_page_width); // HPDF_Page_GetWidth(h_current_page);
	f_length = INCH2PT(f_page_length); // HPDF_Page_GetHeight(h_current_page);

	f_xpos = f_margin_x = INCH2PT(f_margin_left);
	f_margin_y = INCH2PT(f_margin_top);
	f_ypos = f_length - f_margin_y;

	HPDF_Page_SetHeight(h_current_page, f_length);
	HPDF_Page_SetWidth(h_current_page, f_width);

	f_linespace = 0;

	HPDF_Page_BeginText(h_current_page);

}

void hpdf_doc::end_page() 
{
	HPDF_Page_EndText(h_current_page);

	h_current_page = 0;
}

void hpdf_doc::protrait()
{
	if (h_direction == HPDF_PAGE_PORTRAIT)
		return;

	h_direction = HPDF_PAGE_LANDSCAPE;
	HPDF_Page_SetSize(h_current_page, HPDF_PAGE_SIZE_A4, h_direction);

	f_width = INCH2PT(f_page_width); 
	f_length = INCH2PT(f_page_length); 

	HPDF_Page_SetHeight(h_current_page, f_length);
	HPDF_Page_SetWidth(h_current_page, f_width);

}

void hpdf_doc::landscape()
{
	if (h_direction == HPDF_PAGE_LANDSCAPE)
		return;

	h_direction = HPDF_PAGE_PORTRAIT;

	f_length = INCH2PT(f_page_width);
	f_width = INCH2PT(f_page_length);

	HPDF_Page_SetHeight(h_current_page, f_length);
	HPDF_Page_SetWidth(h_current_page, f_width);

}

void hpdf_doc::rotate_text(HPDF_REAL xpos, HPDF_REAL ypos, char* out_string)
{
	/*
	* Rotating text
	*/
	HPDF_REAL angle1 = 90;                   /* A rotation of 30 degrees. */
	HPDF_REAL rad1 = (HPDF_REAL) (angle1 / 180 * 3.141592); /* Calcurate the radian value. */

	HPDF_Page_SetTextMatrix(h_current_page, cos(rad1), sin(rad1), -sin(rad1), cos(rad1), xpos, ypos);
	HPDF_Page_ShowText(h_current_page, out_string);

}

void hpdf_doc::vertical(HPDF_REAL xpos, HPDF_REAL ypos)
{
	HPDF_REAL angle = 90;                   /* A rotation of 90 degrees. */
	HPDF_REAL rad = (HPDF_REAL) (angle / 180 * 3.141592); /* Calcurate the radian value. */

	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;

	HPDF_REAL font_height = MMTEXT2PTX(Z * n_TY);
	HPDF_REAL font_width = MMTEXT2PTX(W * n_TX);

	HPDF_Page_SetTextMatrix(h_current_page, cos(rad), sin(rad), -sin(rad), cos(rad), xpos, ypos);
}

void hpdf_doc::horizontal(HPDF_REAL xpos, HPDF_REAL ypos)
{
	HPDF_REAL angle = 0;                   /* A rotation of 90 degrees. */
	HPDF_REAL rad = (HPDF_REAL) (angle / 180 * 3.141592); /* Calcurate the radian value. */

	HPDF_Page_SetTextMatrix(h_current_page, cos(rad), sin(rad), -sin(rad), cos(rad), xpos, ypos);
}

void hpdf_doc::add_text(et_type datatype, wstring out_string)
{
	char *line = new char[4096];
	memset(line, 0, 4096);
	_locale_t loceng;
	size_t size = 0;

	loceng = _create_locale(LC_ALL, "en-US");

	int len = out_string.length(); // count of space
	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;

	HPDF_REAL f_advance = 0.0;
	HPDF_REAL f_width = MMTEXT2PTX(W * n_TX / 2);
	HPDF_REAL f_gap = MMTEXT2PTX(X/2);
	HPDF_REAL f_space = MMTEXT2PTY((Z * n_TY) + L);

	if (f_space > f_linespace)
		f_linespace = f_space;

	select_datatype_font(datatype);

	switch (datatype) {
	case ET_LATAN:
		
		/*
		size = _wcstombs_l(line, out_string.c_str(), 4096, loceng);
		if (size == 0) goto END_PROC;

		HPDF_Page_TextOut(h_current_page, f_xpos, f_ypos - f_linespace, line);
		f_advance = HPDF_Page_TextWidth(h_current_page, line); 
		*/
		
		text_out_eng(f_xpos, f_ypos, out_string, f_advance, f_width, f_gap, f_space, loceng);
		break;
	case ET_SPACE:
		f_advance += ((f_width + f_gap) * len);
		break;

	case ET_CJK:
	case ET_CJKFORM:
	case ET_BOXDRAW:

		if (VorH == 'H' || datatype != ET_CJK)
			horizontal(f_xpos, f_ypos);
		else
			vertical(f_xpos, f_ypos);

		if (datatype == ET_BOXDRAW) resize_font_boxdraw();
		/*
		size = wchar_to_utf8(out_string.c_str(), out_string.length(), line, 4096, NULL);
		if (size == 0) goto END_PROC;
		
		HPDF_Page_TextOut(h_current_page, f_xpos, f_ypos - f_linespace, line);
		//if (datatype == ET_BOXDRAW)
		//	f_advance += (len * ((f_width + f_gap) * 2));
		//else
			f_advance += HPDF_Page_TextWidth(h_current_page, line); //(len * ((f_width + f_gap) * 2));
			*/
		text_out_cjk(f_xpos, f_ypos, out_string, f_advance, f_width, f_gap, f_space);

		break;
	}

	f_xpos += f_advance;

END_PROC:
	delete [] line; // free buffer

	_free_locale(loceng);
}

void hpdf_doc::new_line()
{
	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;

	HPDF_REAL space = MMTEXT2PTY(Z * n_TY + L);
	if (f_ypos - f_linespace < 0)
	{
		end_page();
		new_page(); // xpos and ypos are reset in the function
	}

	f_xpos = f_margin_x;
	f_ypos -= f_linespace;

	f_linespace = space;
}

void hpdf_doc::text_goto(int px, int py)
{
	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;

	HPDF_REAL space = MMTEXT2PTY(Z * n_TY + L);
	if (space > f_linespace) f_linespace = space;

	HPDF_REAL fpx = ((HPDF_REAL)px * 72.0 / n_log_X);
	HPDF_REAL fpy = ((HPDF_REAL)py * 72.0 / n_log_Y);
	f_xpos = fpx;
	f_ypos = f_length - fpy;

	if (f_ypos < 0)
	{
		HPDF_REAL fy = f_ypos;

		end_page();
		new_page(); // xpos and ypos are reset in the function

		// Move to next page
		f_ypos = fy + f_length;
	}

}

void hpdf_doc::select_datatype_font(et_type datatype)
{
	switch (datatype) {
	case ET_LATAN:
	case ET_SPACE:
		CorE == 'E' ? select_eng_font() : select_cjk_font();
		break;

	case ET_CJKFORM:
	case ET_CJK:
	case ET_BOXDRAW:
		select_cjk_font();
		break;
	}

	if (S > 0) HPDF_Page_SetTextRenderingMode(h_current_page, HPDF_FILL_THEN_STROKE);
	else HPDF_Page_SetTextRenderingMode(h_current_page, HPDF_FILL);

}

void hpdf_doc::select_eng_font()
{
	wstring font_str = StringToWstring(e_ext_font_name);

	if (e_ext_font_name != "")
		select_font(font_str);
	else {
		h_current_font = HPDF_GetFont(h_pdf, PDF_STD_ANSI, NULL);

		if (h_current_page != 0)
			set_font_handle(h_current_page, h_current_font);
	}
}

void hpdf_doc::select_cjk_font()
{
	wstring font_str = StringToWstring(c_ext_font_name);

	if (c_ext_font_name != "")
		select_font(font_str);
	else
		select_font(F);

}

void hpdf_doc::set_font_handle(HPDF_Page h_page, HPDF_Font h_font)
{
	if (h_font != 0) // seccessfully get font
	{
		int n_TX = (TX > 0) ? TX : T,
			n_TY = (TY > 0) ? TY : T;

		HPDF_REAL font_height = MMTEXT2PTX(Z * n_TY);
		HPDF_REAL font_width = MMTEXT2PTX(W * n_TX / 2);
		HPDF_REAL ratio = 100.0;
		/*
		if (font_width >= font_height)
		{
			ratio = (HPDF_REAL) (100.0 * font_width / font_height);
		}
		*/
		ratio = (HPDF_REAL)((double)(200.0 * font_width) / font_height);


		HPDF_Page_SetFontAndSize(h_page, h_font, font_height);
		HPDF_Page_SetHorizontalScalling(h_page, ratio);

		f_font_height = font_height;
		f_font_width = font_width;
	}
}

void hpdf_doc::select_font(int font_index)
{
	wstring font_str(L"A");
     
	if (font_index < 0 || font_index > 25) // invalid index
		return;

	font_str[0] = L'A' + font_index;
	map< std::wstring, mapped_font* >::iterator iter = mapped_font_lookup_table.find(font_str); //find the font index 
	if (iter != mapped_font_lookup_table.end()) {
		h_current_font = iter->second->font_handle;
		set_font_handle(h_current_page, h_current_font);
	}
}


void hpdf_doc::select_font(wstring font_name)
{
	map< std::wstring, mapped_font* >::iterator iter = mapped_font_lookup_table.find(font_name); //find the font index 
	if (iter != mapped_font_lookup_table.end()) {
		h_current_font = iter->second->font_handle;
		set_font_handle(h_current_page, h_current_font);
	}
}

void hpdf_doc::select_font(const char* longfontname)
{
	const char *font_name;
	font_name = HPDF_LoadTTFontFromFile(h_pdf, longfontname, HPDF_TRUE);

	h_current_font = HPDF_GetFont(h_pdf, font_name, PDF_STD_ENCODE);
	set_font_handle(h_current_page, h_current_font);

}

void hpdf_doc::resize_font_boxdraw()
{
	if (h_current_font != 0) // seccessfully get font
	{
		int n_TX = (TX > 0) ? TX : T,
			n_TY = (TY > 0) ? TY : T;

		HPDF_REAL font_height = MMTEXT2PTX((Z * n_TY) + L);
		HPDF_REAL font_width = MMTEXT2PTX((W * n_TX / 2) + (X/2));
		HPDF_REAL ratio = 100.0;
		
		ratio = (HPDF_REAL) ((double)(200.0 * font_width) / font_height);

		HPDF_Page_SetFontAndSize(h_current_page, h_current_font, font_height);
		HPDF_Page_SetHorizontalScalling(h_current_page, ratio);
	}
}


void hpdf_doc::text_out_cjk(HPDF_REAL& f_xpos, HPDF_REAL& f_ypos, wstring out_string, HPDF_REAL& f_advance, HPDF_REAL f_width, HPDF_REAL f_gap, HPDF_REAL f_space)
{
	char line[4];
	memset(line, 0, 4);

	int n_TX = (TX > 0) ? TX : T,
		n_TY = (TY > 0) ? TY : T;
	HPDF_REAL font_height = MMTEXT2PTX((Z * n_TY));
	HPDF_REAL font_width = MMTEXT2PTX((W * n_TX / 2) + (X / 2));
	HPDF_REAL ygap = MMTEXT2PTX(L);
	HPDF_REAL xpos = f_xpos, ypos = f_ypos;

	int len = out_string.length();
	if (VorH == 'V')
	{
		xpos += (f_width + f_gap) * 2;
		ypos -= font_width;
	}

	for (int i = 0; i < len; i++)
	{
		wchar_t out_wchar = out_string[i];
		size_t size = wchar_to_utf8(&out_wchar, 1, line, 4, NULL);
		if (size == 0) break;

		HPDF_Page_TextOut(h_current_page, xpos + f_advance, ypos - f_linespace, line);

		f_advance += (VorH == 'V') ? (font_height) : (f_width + f_gap) * 2;
		//f_advance += (f_width + f_gap) * 2;
	}
}

void hpdf_doc::text_out_eng(HPDF_REAL& f_xpos, HPDF_REAL& f_ypos, wstring out_string, HPDF_REAL& f_advance, HPDF_REAL f_width, HPDF_REAL f_gap, HPDF_REAL f_space, _locale_t loceng)
{
	char line[4];

	int len = out_string.length();

	for (int i = 0; i < len; i++)
	{
		wchar_t out_wchar = out_string[i];
		memset(line, 0, 4);

		size_t size = _wcstombs_l(line, &out_wchar, 1, loceng);
		if (size == 0) break;

		HPDF_Page_TextOut(h_current_page, f_xpos + f_advance, f_ypos - f_linespace, line);
		f_advance += (f_width + f_gap);
	}
}

void hpdf_doc::place_image(int x, int y, int n_destwidth, int n_destlength, int n_srcwidth, int n_srclength, const char *filename)
{
}

void hpdf_doc::polygon(HPDF_Point pt[], int count)
{
	if (count <= 0)
		return;

	HPDF_Page_MoveTo(h_current_page, pt[0].x, pt[0].y);
	for (int i = 1; i < count; i++)
	{
		HPDF_Page_LineTo(h_current_page, pt[i].x, pt[i].y);
	}
	HPDF_Page_LineTo(h_current_page, pt[0].x, pt[0].y);
	HPDF_Page_Fill(h_current_page);
}
