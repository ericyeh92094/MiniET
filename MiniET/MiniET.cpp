// MiniET.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef WIN32
#include <windows.h>
#include <Tlhelp32.h>
#include <winbase.h>
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <wchar.h>
#include <stdlib.h>
#include <process.h>

#include "config.h"
#include "utf8.h"
#include "et_command.h"
#include "et_datachunk.h"
#include "et_hpdfdoc.h"
#include "CmdLineParserEngine.h"

using namespace std;

#define DEFAULT_CCODE			(0x007e)
#define END_CCODE				(0x003b)
#define SOLIDUS					(0x002f)
#define BOM						(0xfeff)
#define LEFT_SQUARE_BRACKET		(0x005b)
#define RIGHT_SQUARE_BRACKET	(0x005d)
#define LEFT_CURLY_BRACKET		(0x007b)
#define RIGHT_CURLY_BRACKET		(0x007d)

#define DEFAULT_CONFIG_FILE		"miniet.config"

bool read_switch(int argc, wchar_t* argv[]);
void read_config(string configfile, wchar_t *envp[]);
int read_file(string filename);
int read_utf8_file(ifstream &fs8, string &firstline);
int read_ansi_file(ifstream &fs8, string &firstline);
int output_pdf_file(string output_file);
void output_pdf_preview(string output_file);
void output_pdf_printing(string output_file);
int break_et_wstring(wchar_t* w_string, size_t str_size, wchar_t control_code, bool new_line);

vector<vector<et_datachunk>> data_vect;
std::vector<et_datachunk> data_line;

static string config_file, target_file, output_file;
static bool b_preview, b_print, b_bom;
static string s_previewer, s_preview_switches, s_printer, s_printer_switches, s_printername, s_drivername, s_portname;
static string s_codepage;
static 	wchar_t basePath[_MAX_PATH];
static wchar_t drive[_MAX_DRIVE];
static wchar_t dir[_MAX_DIR];
static wchar_t fname[_MAX_FNAME];
static wchar_t ext[_MAX_EXT];


int _tmain(int argc, wchar_t* argv[], wchar_t *envp[])
{
	//ShowWindow(GetConsoleWindow(), SW_HIDE);

	command::init_lookup_table(); // init command dispatch table

	if (!read_switch(argc, argv))
		return 0; //switch parsing error

	_wfullpath(basePath, argv[0], sizeof(basePath));

	_wsplitpath(basePath, drive, dir, fname, ext);
	wstring wfont_path_parent = wstring(drive) + wstring(dir); // +L"\"";

	read_config(config_file, envp);

	hpdf_doc::set_fontpath_parent(WstringToString(wfont_path_parent));

	int line = read_file(target_file);

	if (line == 0)
	{
		return -1;
	}
	//start producing the PDF file

	int pdf_line = output_pdf_file(output_file);

	if (b_preview)
		output_pdf_preview(output_file);

	if (b_print)
		output_pdf_printing(output_file);

	//cout << endl << "Press <Enter> to exit this demo... ";
	//cin.get();


	return 0;
}

bool read_switch(int argc,wchar_t* argv[])
{
	b_preview = b_print = false;
	b_bom = true;

	CmdLineParserEngine parser("This program converts ET encoded text to a PDF file");

	CmdLineParserParameter configswitch("configswitch", "c", "set the configuration file");
	configswitch.AddArgument(CmdLineParserArgument("config", "CONFIG"));
	configswitch.AddArgument(CmdLineParserArgument("target", "TARGET"));
	configswitch.AddArgument(CmdLineParserArgument("output", "OUTPUT"));

	parser.AddParameter(configswitch);

	parser.AddExample(CmdLineParserExample("", "Switch usage: -c config target output"));
	parser.AddExample(CmdLineParserExample("-c", "config target output"));

	CmdLineParserParameter previewswitch("previewswitch", "v", "preview the output file");
	parser.AddParameter(previewswitch);

	CmdLineParserParameter printswitch("printswitch", "p", "print the output file");
	parser.AddParameter(printswitch);

	CmdLineParserParameter cpswitch("cpswitch", "cp", "code page setting");
	cpswitch.AddArgument(CmdLineParserArgument("codepage", "CODEPAGE"));

	parser.AddParameter(cpswitch);

	CmdLineParserParameter bomswitch("bomswitch", "nobom", "UTF BOM setting");
	parser.AddParameter(bomswitch);

	if (parser.ProcessCommandLine(argc, argv) == false)
	{
		cerr << "Failed to parse the command line." << endl;
		goto end_block;
	}

	// logic is here under
	if ((parser.GetParsedParametersCount() == 0) && (!parser.IsHelpRequested())) // no switch found
	{  
		
		if (argc != 3) {
			cout << "\nUsage: MiniET filename outputPDF\n";
			return false;
		}
		wstring wstr;
		wstr = argv[1];
		target_file = string(wstr.begin(), wstr.end());

		wstr = argv[2];
		output_file = string(wstr.begin(), wstr.end());

	}
	else
	{
		CmdLineParserParameter param;
		CmdLineParserArgument argument;

		if (parser.TryGetParameter("configswitch", param))
		{
			if (param.TryGetArgument("config", argument))
			{
				// cout << "config file: " << argument.m_sContent << endl;
				config_file = argument.m_sContent;
			}
			if (param.TryGetArgument("target", argument))
			{
				//cout << "target file name:" << argument.m_sContent << endl;
				target_file = argument.m_sContent;
			}
			if (param.TryGetArgument("output", argument))
			{
				//cout << "output file name:" << argument.m_sContent << endl;
				output_file = argument.m_sContent;
			}

		}
		if (parser.TryGetParameter("previewswitch", param))
		{
			b_preview = true;
		}
			
		if (parser.TryGetParameter("printswitch", param))
		{
			b_print = true;
		}

		if (parser.TryGetParameter("cpswitch", param))
		{
			if (param.TryGetArgument("codepage", argument))
				s_codepage = "." + argument.m_sContent;
		}

		if (parser.TryGetParameter("bomswitch", param))
		{
			b_bom = false;
		}

	}
end_block:

	return true;

}

void read_config(string configfile, wchar_t *envp[])
{
	if (configfile == "")
		configfile = DEFAULT_CONFIG_FILE;

	Config config(configfile, envp);

	map<string, Config*> groups = config.getGroups(); // all groups

	for (map<string, Config*>::iterator i = groups.begin(); i != groups.end(); ++i)
	{
		string groupName = i->first;
		Config* group = i->second;

		if (groupName == "Paper") {
			HPDF_REAL width, length, left, top, right, bottom;

			width = group->pDouble("width");
			length = group->pDouble("length");
			left = group->pDouble("left");
			top = group->pDouble("top");
			right = group->pDouble("right");
			bottom = group->pDouble("bottom");

			hpdf_doc::set_paper_margins(width, length, top, left, right, bottom);
		}
		
		if (groupName == "Fonts")
		{
			map<string, string>symbol = group->getSymbols();
			string font_name, font_path;

			for (map<string, string>::iterator j = symbol.begin(); j != symbol.end(); ++j)
			{
				font_name = j->first;
				if (font_name == "Folder")
				{
					hpdf_doc::set_fontpath_parent(j->second);
				}
				else {
					font_path = j->second;
					hpdf_doc::add_external_font(font_name, font_path);
				}
			}		
		}

		if (groupName == "Preview")
		{
			s_previewer = group->pString("program");
			s_preview_switches = group->pString("switches");
		}

		if (groupName == "Printing")
		{
			s_printer = group->pString("program");
			s_printer_switches = group->pString("switches");
			s_printername = group->pString("printername");
			s_drivername = group->pString("drivername");
			s_portname = group->pString("portname");
		}
	}

}

#ifdef WIN32
void killProcessByName(const wchar_t *filename)
{
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	_wsplitpath(filename, drive, dir, fname, ext);
	wcscat(fname, ext);


	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (wcscmp(pEntry.szExeFile, fname) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}
#endif

void output_pdf_preview(string output_file)
{
#ifndef WIN32
	string cmd_line = "\"\"" + s_previewer + "\" " + s_preview_switches + " " + output_file + "\"";

	system(cmd_line.c_str());
#else
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	wstring cmd_line = L"";

	if (s_preview_switches == "\n")
		cmd_line = L"\"" + StringToWstring(s_previewer) + L"\" " + StringToWstring(output_file);
	else
		cmd_line = L"\"" + StringToWstring(s_previewer) + L"\" " + StringToWstring(s_preview_switches) + L" " + StringToWstring(output_file);

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		(LPWSTR) cmd_line.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

#endif
}

void output_pdf_printing(string output_file)
{
#ifndef WIN32
	string cmd_line = "\"\"" + s_printer + "\" " + s_printer_switches + " " + output_file +" \"" + s_printername + "\" \"" + s_drivername + "\" \"" + s_portname + "\"\"";

	system(cmd_line.c_str());

#else
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;  // or SW_SHOWNORMAL or SW_MINIMIZE

	ZeroMemory(&pi, sizeof(pi));

	wstring cmd_line = L"\"" + StringToWstring(s_printer) + L"\" " + StringToWstring(s_printer_switches) + L" " + StringToWstring(output_file)
	+ L" \"" + StringToWstring(s_printername) + L"\" \"" + StringToWstring(s_drivername) + L"\" \"" + StringToWstring(s_portname) + L"\"";

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		(LPWSTR)cmd_line.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, 10000);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	killProcessByName(StringToWstring(s_printer).c_str());
#endif
}

int read_file(string filename)
{
	ifstream fs8(filename);
	if (!fs8.is_open()) {
		cout << "Could not open " << filename << endl;
		return 0;
	}
	string line;
	int result = 0;
	bool isUTF8 = false;
	if (s_codepage == ".65001") isUTF8 = true;

	if (getline(fs8, line)) // get the first line to test the signature
	{
		unsigned char c0 = (unsigned char)line[0];
		unsigned char c1 = (unsigned char)line[1];
		unsigned char c2 = (unsigned char)line[2];

		if ((c0 == 0xef && c1 == 0xbb && c2 == 0xbf) || isUTF8) // UTF - 8
		{
			result = read_utf8_file(fs8, line);
		}
		else // ansi
		{
			result = read_ansi_file(fs8, line);
		}
	}

	return result;
}

int read_utf8_file(ifstream &fs8, string &firstline)
{
	data_vect.clear();

	unsigned line_count = 1;
	string line = firstline;
	int utf_flag = b_bom ? 0 : UTF8_SKIP_BOM;

	// Play with all the lines in the file
	do {

		wchar_t *wchar_mem;
		size_t slen = line.length();

		if (slen > 0) {

			size_t act_size = utf8_to_wchar(line.c_str(), slen, NULL, 0, 0); // calc the size
			wchar_mem = new wchar_t[act_size];
			if (wchar_mem == NULL) break; // something wrong ...

			size_t size = utf8_to_wchar(line.c_str(), slen, wchar_mem, act_size, utf_flag); // convert to unicode
			if (act_size == size)
			{
				//scanning
				int result = break_et_wstring(wchar_mem, act_size, DEFAULT_CCODE, true);
			}
			else {
				cout << "UTF8 parsing error\n";
				slen = -1; //force break
			}

			delete[] wchar_mem;
		}

		line_count++;

	} while (getline(fs8, line));

	fs8.close();

	return line_count;
}

int read_ansi_file(ifstream &fs8, string &firstline)
{
	data_vect.clear();

	unsigned line_count = 1;
	string line = firstline, language_str = "zh-TW";

	if (s_codepage.length() > 0)
		language_str = s_codepage;

	_locale_t loctw;
	loctw = _create_locale(LC_ALL, language_str.c_str());

	// Play with all the lines in the file
	do {

		wchar_t *wchar_mem;
		size_t slen = line.length();

		if (slen > 0) {

			size_t act_size = _mbstowcs_l(NULL, line.c_str(), 0, loctw); // calc the size
			wchar_mem = new wchar_t[act_size + 1];
			if (wchar_mem == NULL) break; // something wrong ...

			size_t size = _mbstowcs_l(wchar_mem, line.c_str(), act_size, loctw); // convert to unicode
			if (act_size == size)
			{
				//scanning
				int result = break_et_wstring(wchar_mem, act_size, DEFAULT_CCODE, true);
			}
			else {
				cout << "Ansi parsing error\n";
				slen = -1; //force break
			}

			delete[] wchar_mem;
		}

		line_count++;

	} while (getline(fs8, line));

	fs8.close();

	return line_count;
}

void prepare_line(vector<et_datachunk> &local_data_line, hpdf_doc *doc)
{
	vector<et_datachunk>::iterator data_iter;
	et_controls cp = doc->et_cp;

	data_iter = local_data_line.begin();
	for (; data_iter != local_data_line.end(); data_iter++)
	{
		et_datachunk &dc = *data_iter;
		bool result = false;

		if (dc.type == ET_COMMAND || dc.type == ET_SP_COMMAND)
		{
			dc.cp = cp;
			result = command::dispatch_command(*doc, dc);
			cp = dc.cp;
		}
		else if (dc.type == ET_NEWPAGE)
		{
		}
		else if (dc.type == ET_DRAWBOX) {
			// dc.rect had been calc
		}
		else { // paint-able elements
			dc.cp = cp;
			if (dc.cp.Q > 0)
			{
				wstring spaces(dc.cp.Q, L' ');
				dc.w_string = spaces + dc.w_string;
			}
			doc->calc_text(dc);
		}
	}
	doc->et_cp = cp;
}

void calc_line(vector<et_datachunk> &local_data_line, hpdf_doc *doc)
{
	vector<et_datachunk>::iterator data_iter;
	et_controls cp = doc->et_cp;
	HPDF_REAL f_line_width = 0, f_line_height = 0;

	data_iter = local_data_line.begin();
	for (; data_iter != local_data_line.end(); data_iter++)
	{
		et_datachunk &dc = *data_iter;
		
		if (dc.type != ET_COMMAND && dc.type != ET_SP_COMMAND && dc.type != ET_DRAWBOX) {
			HPDF_REAL width = fabs(dc.rect.right - dc.rect.left);
			HPDF_REAL height = fabs(dc.rect.top - dc.rect.bottom);

			f_line_width += width;
			f_line_height = fmax(height, f_line_height);	
		}
	}

	HPDF_REAL f_offset = 0;

	if (f_line_width > 0 && f_line_height > 0)
	{
		HPDF_REAL f_paintable_width = doc->f_width - doc->f_margin_left - doc->f_margin_right;

		switch(doc->et_cp.J)
		{ 
			case et_controls::J_LEFT:
				f_offset = 0;
				break;
			case et_controls::J_MIDDLE:
				f_offset = fabs(f_paintable_width - f_line_width) / 2;
				break;
			case et_controls::J_RIGHT:
				f_offset = fabs(f_paintable_width - f_line_width);
				break;
		}
	}

	//adjust the x position
	data_iter = local_data_line.begin();
	for (; data_iter != local_data_line.end(); data_iter++)
	{
		et_datachunk &dc = *data_iter;

		if (dc.type != ET_COMMAND && dc.type != ET_SP_COMMAND && dc.type != ET_DRAWBOX) {
			dc.rect.right += f_offset;
			dc.rect.left += f_offset;
		}
	}
}

int output_pdf_file(string output_file)
{
	vector<vector<et_datachunk>>::iterator line_iter;

	hpdf_doc *doc = new hpdf_doc(output_file.c_str());

	strcpy(doc->source_filename, target_file.c_str());

	line_iter = data_vect.begin();
	doc->begin_doc_and_page();

	for (; line_iter != data_vect.end(); line_iter++)
	{
		vector<et_datachunk> &local_data_line = *line_iter;
		vector<et_datachunk>::iterator data_iter;

		prepare_line(local_data_line, doc); // parse the commands and calc position
		calc_line(local_data_line, doc);

		//Now it is rendering time
		data_iter = local_data_line.begin();
		for (; data_iter != local_data_line.end(); data_iter++)
		{
			et_datachunk &dc = *data_iter;
			bool result = false;

			if (dc.type == ET_COMMAND || dc.type == ET_SP_COMMAND)
			{
				//result = command::dispatch_command(*doc, dc);
				doc->et_cp = dc.cp;
				
			}
			else if (dc.type == ET_NEWPAGE)
			{
				doc->end_page();
				doc->new_page();
			}
			else if (dc.type == ET_DRAWBOX) {
				doc->place_image(dc.rect.top, dc.rect.left, dc.rect.bottom, dc.rect.right, dc.ev, dc.w_string);
			}
			else { // paint-able elements
				doc->et_cp = dc.cp; // carry over all controls
				doc->add_text(dc);			
			}

		}
		doc->new_line();
	}
	doc->end_doc();

	return 0;
}

size_t process_command(vector<et_datachunk> &data_line, wstring w_str, size_t start, size_t str_size)
{
	bool in_processing = true;
	wstring w_string(w_str);
	wstring command_string;

	do {
		if (w_string[start] == LEFT_SQUARE_BRACKET) // process field command
		{
			size_t end = w_string.find(RIGHT_SQUARE_BRACKET, start);
			if (end != string::npos)
			{
				command_string = w_string.substr(start + 1, (end - start));
				if (!command_string.empty())
				{
					et_datachunk dc(ET_SP_COMMAND); // new chunk
					dc.w_string = command_string;
					data_line.push_back(dc);

					command_string.clear();
				}
				start = end + 1;
			}
		}
		
		if (w_string[start] == LEFT_CURLY_BRACKET) // process frame command
		{
			size_t end = w_string.find(RIGHT_CURLY_BRACKET, start);
			if (end != string::npos)
			{
				command_string = w_string.substr(start + 1, (end - start + 1));
				if (!command_string.empty() && command_string[0] == 'U')
				{
					wchar_t cprefix[2];
					cprefix[0] = DEFAULT_CCODE;
					cprefix[1] = 0;

					command_string.insert(0, cprefix);
					break_et_wstring((wchar_t*)command_string.data(), (end - start), DEFAULT_CCODE, false); // recursive call

					command_string.clear();
				}
				else {

					et_datachunk dc(ET_COMMAND); // new chunk
					dc.w_string = command_string;
					data_line.push_back(dc);

					command_string.clear();
				}
				start = end + 1;
			}
		}

		if (w_string[start] == END_CCODE || w_string[start] == DEFAULT_CCODE)
		{
			if (!command_string.empty()) {

				et_datachunk dc(ET_COMMAND); // new chunk
				dc.w_string = command_string;
				data_line.push_back(dc);

				command_string.clear();
			}
			if (w_string[start] == END_CCODE) start++;
			in_processing = false;
		}
		else {
			command_string.append(&w_string[start], 1);
			start++;
		}
		if (start >= str_size)
		{
			if (!command_string.empty()) {

				et_datachunk dc(ET_COMMAND); // new chunk
				dc.w_string = command_string;
				data_line.push_back(dc);

				command_string.clear();
			}
			in_processing = false;
		}

	} while (in_processing);

	return start;
}

size_t process_datachunk(vector<et_datachunk> &data_line, wstring &w_string, size_t start,size_t str_size, et_type type)
{
	int(*isType)(wchar_t);

	switch (type)
	{
	case ET_CJKFORM:
		isType = isCJKFORM;
		break;
	case ET_CJK:	
		isType = isCJK;
		break;
	case ET_BOXDRAW:
		isType = isBOXDRAW;
		break;
	case ET_SPACE:
		isType = isSPACE;
		break;
	default:
		isType = isBASICLATAN;
	};

	et_datachunk *dc = 0;
	dc = new et_datachunk(type); // new chunk
	do {
		dc->w_string.append(&w_string[start], 1);
		start++;
	} while ((*isType)(w_string[start]) && (w_string[start] != DEFAULT_CCODE) && start < str_size);

	data_line.push_back(*dc);
	delete dc;

	return start;
}

int break_et_wstring(wchar_t *w_str, size_t str_size, wchar_t control_code, bool new_line = true)
{
	size_t start = 0;
	int da_count = 0, co_count = 0;

	if (new_line)
		data_line.clear();

	wstring w_string(w_str);

	while (start < str_size)
	{
		wchar_t cp = w_string[start];
		if (cp == BOM)
		{
			start++;
			continue;
		}

		if (cp == L'\f') // form feed
		{
			et_datachunk *dc = new et_datachunk(ET_NEWPAGE); // new chunk
			dc->w_string = L"\f";
			data_line.push_back(*dc);

			start++;
			continue;
		}

		if (control_code == cp)
		{
			if (start > 1 && w_string[start - 1] == SOLIDUS)
			{
				// skip it since it's part of the file path, assuming
				goto LATAN_PROC;
			}
			// command processing
			start = process_command(data_line, w_string, start + 1, str_size);
			co_count++;
			
		}

		else if (isCJKFORM(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_CJKFORM);
			da_count++;
		}
		else if (isCJK(cp) || isFULLWIDTHSPACE(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_CJK);
			da_count++;
		}
		else if (isBOXDRAW(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_BOXDRAW);
			da_count++;
		}
		else if (isSPACE(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_SPACE);
			da_count++;
		}
		else /* if (isBASICLATAN(cp)) */
		{
		LATAN_PROC:
			start = process_datachunk(data_line, w_string, start, str_size, ET_LATAN);
			da_count++;
		}
	}

	bool result = false;
	if (new_line)
	{
		data_vect.push_back(data_line); // a new line
	}
	return result;
}

