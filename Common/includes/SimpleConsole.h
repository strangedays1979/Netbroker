#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <io.h>
#include <fcntl.h>

#include "windows.h"

namespace Console
{
	void ON()
	{
		FreeConsole();
		AllocConsole();

		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt_out = _open_osfhandle((long)handle_out, _O_TEXT);
		FILE* hf_out = _fdopen(hCrt_out, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;

		HANDLE handle_err = GetStdHandle(STD_ERROR_HANDLE);
		int hCrt_err = _open_osfhandle((long)handle_err, _O_TEXT);
		FILE* hf_err = _fdopen(hCrt_err, "w");
		setvbuf(hf_err, NULL, _IONBF, 1);
		*stderr = *hf_err;
	}

	void OFF()
	{
		FreeConsole();
	}
};

#endif // _CONSOLE_H
