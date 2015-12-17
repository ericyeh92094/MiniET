#ifndef _ET_HPDFDOC_
#define _ET_HPDFDOC_

#include <string>
#include <wchar.h>
#include <map>
#include "hpdf.h"

#include "et_datachunk.h"

#define PDF_STD_ANSI    "Times-Roman"
#define PDF_STD_CHS     "STSong-Light"
#define PDF_STD_CHT     "MingLiU"
#define PDF_STD_JPN     "HeiseiMin-W3"
#define PDF_STD_KOR     "HYSMyeongJo-Medium"

#define PDF_ENCODE_ANSI		"host"
#define PDF_ENCODE_CHS_H	"GB-EUC-H"
#define PDF_ENCODE_CHT_H	"ETen-B5-H"
#define PDF_ENCODE_JPN_H	"90ms-RKSJ-H"
#define PDF_ENCODE_KOR_H	"KSCms-UHC-HW-H"

#define PDF_ENCODE_CHS_V	"GB-EUC-V"
#define PDF_ENCODE_CHT_V	"ETen-B5-V"
#define PDF_ENCODE_JPN_V	"90ms-RKSJ-V"
#define PDF_ENCODE_KOR_V	"KSCms-UHC-HW-V"

#define PDF_STD_UNICODE		"Fonts\\GenShinGothic-Monospace-Normal.ttf"
#define PDF_STD_BOLD		"Fonts\\GenShinGothic-Monospace-Bold.ttf"
#define PDF_STD_KAI			"Fonts\\kaiu.ttf"
#define PDF_STD_ENCODE		"UTF-8"

#define PDF_DEFAULT_MARGIN_TOP		(1.0)
#define PDF_DEFAULT_MARGIN_LEFT		(0.5)
#define PDF_DEFAULT_MARGIN_BOTTOM	(1.5)
#define PDF_DEFAULT_MARGIN_RIGHT	(1.5)
#define PDF_DEFAULT_WIDTH			(210/25.4) // inch - A4
#define PDF_DEFAULT_LENGTH			(297/25.4) // inch - A4

using namespace std;

typedef void(*draw_symbol_function)(int line_width, int lw, int x, int y, int x1, int y1, int x2, int y2, int w, int h, int w1, int h1, int w2, int h2);

class mapped_font {
public:
	char	font_name[_MAX_FNAME];
	char	font_encoding[_MAX_FNAME];
	HPDF_Font	font_handle;

	mapped_font(const char*name, const char* encoding) { strcpy(font_name, name); strcpy(font_encoding, encoding); font_handle = NULL; };
};


class hpdf_doc {

public:
	static HPDF_REAL def_f_margin_top;
	static HPDF_REAL def_f_margin_left;
	static HPDF_REAL def_f_margin_bottom;
	static HPDF_REAL def_f_margin_right;
	static HPDF_REAL def_f_length;
	static HPDF_REAL def_f_width;

	static string font_path_parent;

	typedef void(*draw_symbol_function)(int line_width, int lw, int x, int y, int x1, int y1, int x2, int y2, int w, int h, int w1, int h1, int w2, int h2);

	inline HPDF_REAL MMTEXT2PTX(long nUnit) { return (HPDF_REAL)((double)nUnit * 72.0 / (double)n_log_X); }
	inline HPDF_REAL MMTEXT2PTY(long nUnit) { return (HPDF_REAL)((double)nUnit * 72.0 / (double)n_log_Y); }
	inline HPDF_REAL D2PTX(long nUnit) { return (HPDF_REAL)((double)nUnit * 72.0 / ((double)n_log_X * 1000L)); }
	inline HPDF_REAL D2PTY(long nUnit) { return (HPDF_REAL)((double)nUnit * 72.0 / ((double)n_log_Y * 1000L)); }

	inline HPDF_REAL INCH2PT(float fUnit) { return (HPDF_REAL)(fUnit * 72.0); }

	map< std::wstring, mapped_font* > mapped_font_lookup_table;
	map< unsigned char, draw_symbol_function> mapped_draw_function_table;

	static map< std::string, string> external_fonts;

public:
	char filename[_MAX_PATH];
	char source_filename[_MAX_PATH];

	HPDF_Doc h_pdf;
	HPDF_Page h_current_page;
	HPDF_Font h_current_font;
	HPDF_Font h_eng_font;
	HPDF_INT n_log_X, n_log_Y;

	HPDF_REAL f_width, f_length;
	HPDF_REAL f_margin_x, f_margin_y;

	HPDF_REAL f_page_width;
	HPDF_REAL f_page_length;
	HPDF_REAL f_margin_top;
	HPDF_REAL f_margin_left;
	HPDF_REAL f_margin_bottom;
	HPDF_REAL f_margin_right;

	HPDF_REAL f_xpos, f_ypos;
	HPDF_REAL f_linespace;

	HPDF_REAL f_font_height;
	HPDF_REAL f_font_width;

	HPDF_PageSizes	h_size;
	HPDF_PageDirection	h_direction;

	hpdf_doc(const char* filepath);
	~hpdf_doc();

	static void set_paper_margins(HPDF_REAL width, HPDF_REAL length, HPDF_REAL top, HPDF_REAL left, HPDF_REAL bottom, HPDF_REAL right);
	void set_paper_margin(wchar_t margin_indicator, HPDF_INT32 n_margin); // MMTEXT

	static void add_external_font(string font_name, string font_path);
	static void set_fontpath_parent(string font_parent);

	void begin_doc_and_page();
	void end_doc();

	void init_font_table();
	void get_logical_DPI(int pCode, int D, HPDF_INT &LogX, HPDF_INT &LogY, int xRatio, int yRatio);
	void init(int p_code);
	void set_page_size();
	void set_page_orientation();
	void new_page();
	void end_page();
	void protrait();
	void landscape();

	void rotate_text(HPDF_REAL xpos, HPDF_REAL ypos, char* out_string);
	void vertical(HPDF_REAL xpos, HPDF_REAL ypos);
	void horizontal(HPDF_REAL xpos, HPDF_REAL ypos);
	void add_text(et_type datatype, wstring out_string);
	void new_line();
	void text_goto(int px, int py);

	void select_eng_font();
	void select_cjk_font();
	void set_font_handle(HPDF_Page h_page, HPDF_Font h_font);
	void select_datatype_font(et_type datatype);
	void select_font(int index);
	//void select_font(wstring font_letter);
	void select_font(const char* fontname);
	void select_font(wstring fontname);
	void resize_font_boxdraw();
	void text_out_eng(HPDF_REAL& f_xpos, HPDF_REAL& f_ypos, wstring out_string, HPDF_REAL& f_advance, HPDF_REAL f_width, HPDF_REAL f_gap, HPDF_REAL f_space, _locale_t loceng);
	void text_out_cjk(HPDF_REAL& f_xpos, HPDF_REAL& f_ypos, wstring out_string, HPDF_REAL& f_advance, HPDF_REAL f_width, HPDF_REAL f_gap, HPDF_REAL f_space);
	void place_image(int x, int y, int n_destwidth, int n_destlength, int n_srcwidth, int n_srclength, const char *filename);

	void polygon(HPDF_Point pt[], int count);
	//inline void MoveTo(HPDF_REAL x, HPDF_REAL y) { HPDF_Page_MoveTo(h_current_page, MMTEXT2PTX(x), MMTEXT2PTY(y)); };
	//inline void LineTo(HPDF_REAL x, HPDF_REAL y) { HPDF_Page_LineTo(h_current_page, MMTEXT2PTX(x), MMTEXT2PTY(y)); };
	//inline void Arc(HPDF_REAL x1, HPDF_REAL y1, HPDF_REAL x2, HPDF_REAL y2, HPDF_REAL x3, HPDF_REAL y3, HPDF_REAL x4, HPDF_REAL y4) { /*dummy now */ };

	//void draw_box_symbol(unsigned char hi, unsigned char lo, HPDF_REAL left, HPDF_REAL bottom, HPDF_REAL top, HPDF_REAL right, bool b_vert);

	static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data);

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

	string		c_ext_font_name, e_ext_font_name;

	enum JTYPE { J_LEFT, J_MIDDLE, J_RIGHT };

	HPDF_INT	LC; // 0 - 255, -1 = not set

	HPDF_BOOL	HasPosition;
	HPDF_INT	Px, Py, Rx, Ry;

	HPDF_REAL	text_color_r, text_color_g, text_color_b;  // Text color
	HPDF_REAL	table_color_r, table_color_g, table_color_b; // Table Color
	HPDF_REAL	line_color_r, line_color_g, line_color_b;  // Under Line, ~[H]
	HPDF_REAL	bg_color_r, bg_color_g, bg_color_b; // Background Color, ~R

	inline void hpdf_doc::use_eng_font(string font_name)
	{
		e_ext_font_name = font_name;
	}

	inline void hpdf_doc::use_cjk_font(string font_name)
	{
		c_ext_font_name = font_name;
	}
	inline void hpdf_doc::use_font(wstring font_letter)
	{
		F = (int) (font_letter[0] - L'A');
	}

};

inline std::wstring StringToWstring(const std::string str)
{// string?wstring
	unsigned len = str.size() * 2;// ?留字??
	setlocale(LC_CTYPE, "");     //必??用此函?
	wchar_t *p = new wchar_t[len];// 申?一段?存存放??后的字符串
	mbstowcs(p, str.c_str(), len);// ??
	std::wstring str1(p);
	delete[] p;// ?放申?的?存
	return str1;
};

inline std::string WstringToString(const std::wstring str)
{// wstring?string
	unsigned len = str.size() * 4;
	setlocale(LC_CTYPE, "");
	char *p = new char[len];
	wcstombs(p, str.c_str(), len);
	std::string str1(p);
	delete[] p;
	return str1;
};


#endif
