#ifndef _ET_DATACHUNK_
#define _ET_DATACHUNK_

#include <string>
#include <vector>
#include <wchar.h>

typedef enum _et_type_ {
	ET_LATAN, ET_FULLWIDTHSPACE, ET_CJKFORM, ET_CJK, ET_SPACE, ET_BOXDRAW, ET_USER, ET_COMMAND, ET_SP_COMMAND
} et_type;
using namespace std;

struct et_datachunk {
public:
	wstring w_string;
	et_type type;

	et_datachunk():w_string() {};
	et_datachunk(const et_datachunk &dc) { w_string = dc.w_string; type = dc.type; };
	et_datachunk(et_type type);
	et_datachunk& operator = (const et_datachunk &dc) { w_string = dc.w_string; type = dc.type; return *this; };
	~et_datachunk() {};

};

#endif
