
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <stdlib.h>

#include "et_command.h"

int command::get_int(const wchar_t *code, int idx)
{
	int n_start = idx,
		n_end = idx;

	while (code[n_end] && (code[n_end] >= '0' && code[n_end] <= '9') || code[n_end] == '-') // stone 1999/11/19
		n_end++;
	if (n_end == n_start)
		return -1; // no digits found
	else
		return n_end;
}

///////////////////////////////////////////////////////
// cmd_string will be changed.
////////////////////////////////////////////////////////
bool command::process_single_param(wstring& cmd_string, HPDF_INT& param)
{
	int end = 0;
	int len = cmd_string.length();
	if (len > 0)
	{
		end = get_int(cmd_string.c_str(), 1);
		if (end > 0)
		{
			wstring param_str = cmd_string.substr(1, end);
			param = _wtoi(param_str.c_str());

			if (end < len)
			{
				cmd_string = cmd_string.substr(end, len);
				return false;
			}
		}

	}

	return true;
}

/////////////////////////////////////////
struct et_command_I : public command {
public:
	et_command_I() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			doc->init(doc->P); // reset all param
			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}
};

/////////////////////////////////////////
struct et_command_T : public command {
public:
	et_command_T() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->T);
	}

};

/////////////////////////////////////////
struct et_command_P : public command {
public:
	et_command_P() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}
};


//////////////////////////////////////////
struct et_command_S : public command {
public:
	et_command_S() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};

//////////////////////////////////////////
struct et_command_F : public command {
public:
	et_command_F() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};


//////////////////////////////////////////
struct et_command_W : public command {
public:
	et_command_W() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->W);
	}

};


//////////////////////////////////////////
struct et_command_Z : public command {
public:
	et_command_Z() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->Z);
	}

};

//////////////////////////////////////////
struct et_command_X : public command {
public:
	et_command_X() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->X);
	}
};

//////////////////////////////////////////
struct et_command_G : public command {
public:
	et_command_G() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->G);
	}
};

//////////////////////////////////////////
struct et_command_R : public command {
public:
	et_command_R() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};


//////////////////////////////////////////
struct et_command_U : public command {
public:
	et_command_U() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};

//////////////////////////////////////////
struct et_command_L : public command {
public:
	et_command_L() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, doc->L);
	}
};


//////////////////////////////////////////
struct et_command_O : public command {
public:
	et_command_O() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};

//////////////////////////////////////////
struct et_command_V : public command {
public:
	et_command_V() {};
	virtual bool parse(wstring& cmd_string) {
		doc->VorH = 'V';
		return true;
	}

};

//////////////////////////////////////////
struct et_command_H : public command {
public:
	et_command_H() {};
	virtual bool parse(wstring& cmd_string) {
		doc->VorH = 'H';
		return true;
	}

};

//////////////////////////////////////////
struct et_command_QMARK : public command {
public:
	et_command_QMARK() {};
	virtual bool parse(wstring& cmd_string) {
		return true;
	}

};


/////////////////////////////////////////
std::map<std::wstring, command* > command::et_lookup_table;

void command::init_lookup_table()
{
	et_lookup_table[L"I"] = new et_command_I();
	et_lookup_table[L"T"] = new et_command_T();
	et_lookup_table[L"P"] = new et_command_P();
	et_lookup_table[L"S"] = new et_command_S();
	et_lookup_table[L"F"] = new et_command_F();
	et_lookup_table[L"W"] = new et_command_W();
	et_lookup_table[L"X"] = new et_command_X();
	et_lookup_table[L"Z"] = new et_command_Z();
	et_lookup_table[L"G"] = new et_command_G();
	et_lookup_table[L"U"] = new et_command_R();
	et_lookup_table[L"U"] = new et_command_U();
	et_lookup_table[L"L"] = new et_command_L();
	et_lookup_table[L"O"] = new et_command_O();
	et_lookup_table[L"H"] = new et_command_H();
	et_lookup_table[L"V"] = new et_command_V();
	et_lookup_table[L"?"] = new et_command_QMARK();
}

bool command::dispatch_command(hpdf_doc &doc, et_datachunk& dc) //
{
	bool result = false;

	wstring command_string;
	command_string = dc.w_string;

	while (1)
	{		
		wstring cmd_name = command_string.substr(0, 1);
		cmd_name[0] = ::towupper(cmd_name[0]);

		std::map< std::wstring, command* >::iterator iter = et_lookup_table.find(cmd_name); //find the command object 
		if (iter != et_lookup_table.end()) {
			iter->second->datachunk = &dc;
			iter->second->doc = &doc;
			result = iter->second->parse(command_string);
			if (result)
				break;
		}
		else
		{
			int len = command_string.length();
			if (len > 1)
				command_string = command_string.substr(1, len); // skip the unfound command
			else
				break; // reach the end of the string
		}
	}

	return result;
}