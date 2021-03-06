
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <stdlib.h>

#include "et_command.h"

extern vector<vector<et_datachunk>> data_vect;
extern std::vector<et_datachunk> data_line;

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
			int P = datachunk->cp.P;
			doc->init(datachunk->cp.P); // reset all param
			datachunk->cp.init(P);

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
		return process_single_param(cmd_string, datachunk->cp.T);
	}

};

/////////////////////////////////////////
struct et_command_U : public command {
public:
	et_command_U() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.U);
	}

};

/////////////////////////////////////////
struct et_command_P : public command {
public:
	et_command_P() {};
	virtual bool parse(wstring& cmd_string) {
		HPDF_INT P = 0;
		bool result = process_single_param(cmd_string, P);

		if (P < 60)
			datachunk->cp.P = 9;
		else if (P < 100)
			datachunk->cp.P = 60;
		else
			datachunk->cp.P = P;

		doc->init(datachunk->cp.P); // reset all param
		datachunk->cp.init(P);

		return result;
	}
};

/////////////////////////////////////////
struct et_command_SP_P : public command { // [P;dd;dd]
public:
	et_command_SP_P() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			wstring code_str = cmd_string.substr(1, len);
			if (code_str[0] == L';')
			{
				int px = 0, py = 0;
				if (swscanf(code_str.c_str(), L";%d;%d", &px, &py) == 2)
				{
					doc->text_goto(px, py, datachunk->cp);
				}
			}
			
		}
		return true;
	}
};

/////////////////////////////////////////
struct et_command_SP_G : public command { // [G;filepath;x;y;w;h;ev]
public:
	et_command_SP_G() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			wstring code_str = cmd_string.substr(1, len);
			if (code_str[0] == L';')
			{
				int pos = code_str.find(L';', 1);
				wstring file_path = code_str.substr(1, pos - 1);
				
				code_str = code_str.substr(pos + 1, code_str.length() - pos - 2);
				int px = 0, py = 0, w = 0, h = 0, ev = 0;
				if (swscanf(code_str.c_str(), L"%d;%d;%d;%d;%d", &px, &py, &w, &h, &ev) == 5)
				{
					doc->place_image(px, py, px + w, py + h, ev, file_path);			
					/*
					et_datachunk dc(ET_DRAWBOX); // new chunk
					dc.w_string = file_path;
					dc.rect.top = px;
					dc.rect.left = py;
					dc.rect.right = px + w;
					dc.rect.bottom = py + h;
					data_line.push_back(dc);
					*/
				}
			}

		}
		return true;
	}
};
//////////////////////////////////////////
struct et_command_F : public command {
public:
	et_command_F() {};
	virtual bool parse(wstring& cmd_string) {

		int len = cmd_string.length();
		if (len > 0)
		{
			wstring font_letter = cmd_string.substr(1, 1);
			if (font_letter[0] == L';')
			{
				// font name
				wstring font_wstr = cmd_string.substr(2, len-3);
				string font_name = string(font_wstr.begin(), font_wstr.end());
				datachunk->cp.use_eng_font(font_name);
				datachunk->cp.use_cjk_font(font_name);

				len = 0;
			}
			else
			{
				datachunk->cp.F = datachunk->cp.use_font(font_letter);
			}

			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_CF : public command {
public:
	et_command_CF() {};
	virtual bool parse(wstring& cmd_string) {

		int len = cmd_string.length();
		if (len > 0)
		{
			wstring font_letter = cmd_string.substr(2, 1);
			if (font_letter[0] == L';')
			{
				// font name
				wstring font_wstr = cmd_string.substr(3, len - 4);
				string font_name = string(font_wstr.begin(), font_wstr.end());
				datachunk->cp.use_cjk_font(font_name);
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_EF : public command {
public:
	et_command_EF() {};
	virtual bool parse(wstring& cmd_string) {

		int len = cmd_string.length();
		if (len > 0)
		{
			wstring font_letter = cmd_string.substr(2, 1);
			if (font_letter[0] == L';')
			{
				// font name
				wstring font_wstr = cmd_string.substr(3, len - 4);
				string font_name = string(font_wstr.begin(), font_wstr.end());
				datachunk->cp.use_eng_font(font_name);
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_MX : public command {
public:
	et_command_MX() {};
	virtual bool parse(wstring& cmd_string) {

		int len = cmd_string.length();
		if (len > 0)
		{
			wstring delim = cmd_string.substr(2, 1);
			wstring margin_indicator = cmd_string.substr(1, 1); // L, T, R, B
			wstring margin_str = cmd_string.substr(3, len);
			long n_margin = _wtol(margin_str.c_str());

			if (delim[0] == L';')
			{
				doc->set_paper_margin(margin_indicator[0], (HPDF_INT32)n_margin);
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_JX : public command {
public:
	et_command_JX() {};
	virtual bool parse(wstring& cmd_string) {

		int len = cmd_string.length();
		if (len > 0)
		{
			wstring just_indicator = cmd_string.substr(1, 1); // L, M, R

			if (just_indicator[0] == 'L')
				datachunk->cp.J = et_controls::J_LEFT;
			if (just_indicator[0] == 'M')
				datachunk->cp.J = et_controls::J_MIDDLE;
			if (just_indicator[0] == 'R')
				datachunk->cp.J = et_controls::J_RIGHT;
		}
		return true;
	}

};


//////////////////////////////////////////
struct et_command_W : public command {
public:
	et_command_W() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.W);
	}

};


//////////////////////////////////////////
struct et_command_Z : public command {
public:
	et_command_Z() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.Z);
	}

};

//////////////////////////////////////////
struct et_command_X : public command {
public:
	et_command_X() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.X);
	}
};

//////////////////////////////////////////
struct et_command_G : public command {
public:
	et_command_G() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.G);
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
struct et_command_S : public command {
public:
	et_command_S() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.S);
	}

};

//////////////////////////////////////////
struct et_command_L : public command {
public:
	et_command_L() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.L);
	}
};


//////////////////////////////////////////
struct et_command_O : public command {
public:
	et_command_O() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.O);
	}

};

//////////////////////////////////////////
struct et_command_Q : public command {
public:
	et_command_Q() {};
	virtual bool parse(wstring& cmd_string) {
		return process_single_param(cmd_string, datachunk->cp.Q);
	}

};

//////////////////////////////////////////
struct et_command_V : public command {
public:
	et_command_V() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			datachunk->cp.VorH = 'V';
			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_H : public command {
public:
	et_command_H() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			datachunk->cp.VorH = 'H';
			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_C : public command {
public:
	et_command_C() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			datachunk->cp.CorE = 'C';
			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_E : public command {
public:
	et_command_E() {};
	virtual bool parse(wstring& cmd_string) {
		int len = cmd_string.length();
		if (len > 0)
		{
			datachunk->cp.CorE = 'E';
			if (len > 1)
			{
				cmd_string = cmd_string.substr(1, len);
				return false;
			}
		}
		return true;
	}

};

//////////////////////////////////////////
struct et_command_J : public command {
public:
	et_command_J() {};
	virtual bool parse(wstring& cmd_string) {
		HPDF_INT J;
		bool result = process_single_param(cmd_string, J);

		if (J == 6)
			doc->protrait();
		else if (J == 7)
			doc->landscape();

		return result;
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
std::map<std::wstring, command* > command::et_sp_lookup_table;


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
	et_lookup_table[L"C"] = new et_command_C();
	et_lookup_table[L"E"] = new et_command_E();
	et_lookup_table[L"J"] = new et_command_J();
	et_lookup_table[L"?"] = new et_command_QMARK();

	et_sp_lookup_table[L"CF"] = new et_command_CF();
	et_sp_lookup_table[L"EF"] = new et_command_EF();
	et_sp_lookup_table[L"P"] = new et_command_SP_P();
	et_sp_lookup_table[L"p"] = new et_command_SP_P();
	et_sp_lookup_table[L"G"] = new et_command_SP_G();
	et_sp_lookup_table[L"g"] = new et_command_SP_G();
	et_sp_lookup_table[L"ML"] = new et_command_MX();
	et_sp_lookup_table[L"MT"] = new et_command_MX();
	et_sp_lookup_table[L"MR"] = new et_command_MX();
	et_sp_lookup_table[L"MB"] = new et_command_MX();
	et_sp_lookup_table[L"JL"] = new et_command_JX();
	et_sp_lookup_table[L"JM"] = new et_command_JX();
	et_sp_lookup_table[L"JR"] = new et_command_JX();


}


bool command::dispatch_command(hpdf_doc &doc, et_datachunk& dc) //
{
	bool result = false;

	wstring command_string;
	command_string = dc.w_string;

	if (dc.type == ET_SP_COMMAND)
	{
		size_t pos = command_string.find(L";", 0);

		if (pos == string::npos) // find no ;
			//return result;
			pos = command_string.length() - 1;

		wstring cmd_name = command_string.substr(0, pos);

		std::map< std::wstring, command* >::iterator iter = et_sp_lookup_table.find(cmd_name); //find the command object 
		if (iter != et_sp_lookup_table.end()) {
			iter->second->datachunk = &dc;
			iter->second->doc = &doc;
			result = iter->second->parse(command_string);
		}

		return result;
	}

	while (1)
	{	
		// single char command
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