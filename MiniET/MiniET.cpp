// MiniET.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <wchar.h>

#include "config.h"
#include "utf8.h"
#include "et_command.h"
#include "et_datachunk.h"
#include "et_hpdcdoc.h"

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

int read_utf8_file(const wchar_t  *filename);
int output_pdf_file(const wchar_t *output_file);
int break_et_wstring(wchar_t* w_string, size_t str_size, wchar_t control_code, bool new_line);

static vector<vector<et_datachunk>> data_vect;
static std::vector<et_datachunk> data_line;

int _tmain(int argc, char* argv[], char *envp[])
{
	command::init_lookup_table(); // init command dispatch table

	if (argc != 3) {
		cout << "\nUsage: MiniET filename outputPDF\n";
		return 0;
	}

	char *test_file_path = argv[1];
	char *output_path= argv[2];



	int line = read_utf8_file(test_file_path);

	if (line == 0)
	{
		cout << "Error reading file\n";
		return 0;
	}
	//start producing the PDF file

	int pdf_line = output_pdf_file(output_path);

	return 0;
}

int read_switch()
{

}

void read_config(string configfile, char *envp[])
{
	if (configfile == "")
		configfile = DEFAULT_CONFIG_FILE;

	Config config(configfile, envp);

	map<string, Config*> groups = config.getGroups(); // all groups

	for (map<string, Config*>::iterator i = groups.begin(); i != groups.end(); ++i) {
		string groupName = i->first;
		Config* group = i->second;

		if (groupName == "Paper") {

		}
		else if (groupName == "Fonts")
	}

}

int read_utf8_file(char *filename)
{
	// Open the test file (contains UTF-8 encoded text)
	ifstream fs8(filename);
	if (!fs8.is_open()) {
		cout << "Could not open " << filename << endl;
		return 0;
	}

	data_vect.clear();

	unsigned line_count = 1;
	string line;


	// Play with all the lines in the file
	while (getline(fs8, line)) {

		wchar_t *wchar_mem;
		size_t slen = line.length();

		if (slen > 0) {

			size_t act_size = utf8_to_wchar(line.c_str(), slen, NULL, 0, 0); // calc the size
			wchar_mem = new wchar_t[act_size];
			if (wchar_mem == NULL) break; // something wrong ...

			size_t size = utf8_to_wchar(line.c_str(), slen, wchar_mem, act_size, 0 /*UTF8_SKIP_BOM*/); // convert to unicode
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
	}

	return line_count;
}

int output_pdf_file(char* output_file)
{
	//char mb_output_file[_MAX_PATH];
	//wcstombs(mb_output_file, output_file,_MAX_PATH);

	vector<vector<et_datachunk>>::iterator line_iter;

	hpdf_doc *doc = new hpdf_doc((const char*)output_file);

	line_iter = data_vect.begin();
	doc->begin_doc_and_page();

	for (; line_iter != data_vect.end(); line_iter++)
	{
		vector<et_datachunk> &local_data_line = *line_iter;
		vector<et_datachunk>::iterator data_iter;

		data_iter = local_data_line.begin();
		for (; data_iter != local_data_line.end(); data_iter++)
		{
			et_datachunk &dc = *data_iter;
			bool result = false;
			if (dc.type == ET_COMMAND)
			{
				result = command::dispatch_command(*doc, dc);
			}
			else { // paint-able elements
				doc->add_text(dc.type, dc.w_string);			

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
				command_string = w_string.substr(start + 1, (end - start + 1));
				if (!command_string.empty())
				{
					et_datachunk dc(ET_COMMAND); // new chunk
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
		if (start > str_size) in_processing = false;

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

		if (control_code == cp)
		{
			if (start > 1 && w_string[start - 1] == SOLIDUS)
			{
				// skip it since it's part of the file path, assuming
				goto LATAN_PROC;
			}
			// command processing
			start = process_command(data_line, w_string, start + 1, str_size);
			
		}

		else if (isCJKFORM(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_CJKFORM);
		}
		else if (isCJK(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_CJK);
		}
		else if (isBOXDRAW(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_BOXDRAW);
		}
		else if (isSPACE(cp))
		{
			start = process_datachunk(data_line, w_string, start, str_size, ET_SPACE);
		}
		else /* if (isBASICLATAN(cp)) */
		{
		LATAN_PROC:
			start = process_datachunk(data_line, w_string, start, str_size, ET_LATAN);
		}
	}

	bool result = false;
	if (new_line)
	{
		data_vect.push_back(data_line); // a new line
	}
	return result;
}

