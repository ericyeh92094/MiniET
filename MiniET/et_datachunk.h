#pragma once

#include <string>
#include <vector>
#include <wchar.h>

#include "hpdf.h"
#include "et_controls.h"

typedef enum _et_type_ {
	ET_LATAN, ET_FULLWIDTHSPACE, ET_CJKFORM, ET_CJK, ET_SPACE, ET_BOXDRAW, ET_DRAWBOX, ET_USER, ET_NEWPAGE, ET_COMMAND, ET_SP_COMMAND
} et_type;
using namespace std;

struct et_datachunk {
public:
	wstring w_string;
	et_type type;
	et_controls cp;
	HPDF_Box rect;
	int	ev;

	et_datachunk():w_string() {};
	et_datachunk(const et_datachunk &dc) { w_string = dc.w_string; type = dc.type; cp = dc.cp; };
	et_datachunk(et_type type);
	et_datachunk& operator = (const et_datachunk &dc) { w_string = dc.w_string; type = dc.type; cp = dc.cp;  return *this; };
	~et_datachunk() {};

};

