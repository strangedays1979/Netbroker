#ifndef _LOAD_LIBRARY_H
#define _LOAD_LIBRARY_H

#include "windows.h"
#include <cstdio>
#include <string>

namespace LIB
{
class Exception
{
public:
	Exception ( const std::string& _message, const std::string& _file,
				const unsigned long& _line ) : m_message(_message), m_file( _file ), m_line( _line ) {};
	std::string getMsg()
	{
		char Buffer[512];	
		sprintf ( Buffer, "[%s:%d] %s", m_file, m_line, m_message );
		return std::string(Buffer);
	}
	void printMsg ( FILE* Stream = stderr )
	{
		fprintf ( Stream, "%s\n", this->getMsg().c_str() );
		fflush(Stream);
	}

private:
	const std::string m_message;
	const std::string m_file;
	const unsigned    m_line;
};		

#define EXCEPTION(EE)																\
class EE : public Exception {														\
public:																				\
	EE (const std::string& _message, const std::string& _file,						\
			const unsigned long& _line ) : Exception(_message,_file,_line) {}; };

EXCEPTION(LibraryException)
class Library
{
private:
	HINSTANCE hinstLib;

public:
	Library() : hinstLib(NULL) {};
	Library ( const char* Name )
	{
		this->Load ( Name );
	}
	~Library()
	{
		this->Free();
	};

	void Load ( const char* Name )
	{
		this->hinstLib = LoadLibrary ( (char*)Name ); 
		if ( NULL == this->hinstLib )
		{
			char Msg[256];
			sprintf ( Msg, "LoadLibrary(%s) failed (%d).", Name, GetLastError() );
			throw LibraryException( Msg, __FILE__, __LINE__ );
		}
	}

	FARPROC getProcAddress ( const char* Name )
	{
		FARPROC Addr = NULL;
		Addr = GetProcAddress ( this->hinstLib, (char*)Name );
		if ( NULL == Addr )
		{
			char Msg[256];
			sprintf ( Msg, "GetProcAddress(%s) failed (%d).", Name, GetLastError() );
			throw LibraryException( Msg, __FILE__, __LINE__ );
		}
		return Addr;
	}

	void Free()
	{
		if ( NULL != this->hinstLib )
		{
			FreeLibrary ( this->hinstLib );
			this->hinstLib = NULL;
		}
	}
};

#define LOAD_LIBRARY(pC,Name,Act)			\
	try									\
	{									\
		pC = new LIB::Library(Name);	\
	}									\
	catch (LIB::LibraryException& E)	\
	{									\
		E.printMsg();					\
		Act;							\
	};

#define GET_PROC_ADDR(pC,Name,F,T,Act)		\
	try										\
	{										\
		F = (T)pC->getProcAddress(Name);	\
	}										\
	catch (LIB::LibraryException& E)		\
	{										\
		E.printMsg();						\
		Act;								\
	};

}
#endif
