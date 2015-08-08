#ifndef _ET_COMMAND_
#define _ET_COMMAND_

#include <iostream>
#include <string>
#include <map>
#include <wchar.h>

#include "et_datachunk.h"
#include "et_hpdcdoc.h"

using namespace std;

//base class for all your commands.
struct command {
public:
	static void init_lookup_table();
	static std::map< std::wstring, command* > et_lookup_table;

	et_datachunk* datachunk;
	hpdf_doc* doc;

	command() { };
	virtual ~command() { };
	virtual bool parse(wstring& cmd_string) = 0; //pure virtual function to execute the commands.

	int get_int(const wchar_t *code, int idx);
	bool process_single_param(wstring& cmd_string, HPDF_INT& param);

	static bool dispatch_command(hpdf_doc& doc, et_datachunk& dc); 
};

#endif

