// EnumPrinters.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <conio.h> 
#include <iostream>
#include <wchar.h>

using namespace std;
void EnumeratePrinters(DWORD flags);

int main()
{
	EnumeratePrinters(PRINTER_ENUM_LOCAL);
	cout << "\nPress any key...";
	_getch();
	
    return 0;
}

void EnumeratePrinters(DWORD flags)
{
	PRINTER_INFO_2* prninfo = NULL;
	DWORD needed, returned;

	//find required size for the buffer
	EnumPrinters(flags, NULL, 2, NULL, 0, &needed, &returned);

	//allocate array of PRINTER_INFO structures
	prninfo = (PRINTER_INFO_2*)GlobalAlloc(GPTR, needed);

	//call again
	if (!EnumPrinters(flags, NULL,
		2, (LPBYTE)prninfo, needed, &needed, &returned))
	{
		cout << "EnumPrinters failed with error code "
			<< GetLastError() << endl;
	}
	else
	{
		cout << "Printers found: " << returned << endl << endl;
		for (int i = 0; i < returned; i++)
		{
			//cout << (char *)prninfo[i].pPrinterName << " on "
			//	<< prninfo[i].pPortName << endl;
			printf("Printer Name:\"%ws\" on \"%ws\" Location %ws\n", (wchar_t *)prninfo[i].pPrinterName, prninfo[i].pPortName, prninfo[i].pLocation);
			printf("Driver Name:\%ws\n", (wchar_t *)prninfo[i].pDriverName);
			if (prninfo[i].pServerName != NULL)
				printf("Server: \"%ws\"\n", prninfo[i].pServerName);

			cout << "Status=" << prninfo[i].Status
				<< ", jobs=" << prninfo[i].cJobs << endl << endl;
		}
	}

	GlobalFree(prninfo);
}