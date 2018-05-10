#ifndef _SYNCHRO_H
#define _SYNCHRO_H

#include "windows.h"
#include <cstdio>
#include <string>
#include <sstream>

#pragma comment(lib,"Winmm.lib")																							

namespace SYNCHRO
{

class Exception
{
public:
	Exception ( const std::string& _message, const std::string& _file,
				const unsigned long& _line ) : m_message(_message), m_file( _file ), m_line( _line ) {};
	std::string getMsg()
	{
		std::ostringstream oss;	
		oss << "[" << m_file << ":" << m_line << "] " << m_message;
		return oss.str();
	}
	void printMsg ( FILE* Stream = stderr )
	{
		printf ( "%s\n", this->getMsg().c_str() );
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

EXCEPTION(TimerException)
class Timer
{
private:
	HANDLE hEvent;
	UINT uTimerID;
	UINT uDelay;

public:
	Timer ( UINT Delay ) throw( TimerException )
	{
		this->hEvent = CreateEvent ( NULL, FALSE, TRUE, NULL ); 
		if ( NULL == this-> hEvent ) 
		{
			char Msg[256];
			sprintf ( Msg, "CreateEvent failure (%d).", GetLastError() );
			throw TimerException( Msg, __FILE__, __LINE__ );
		}

		this->uDelay = Delay;
		this->uTimerID = timeSetEvent( this->uDelay, 0, 
                        (LPTIMECALLBACK)this->hEvent, 0,
                        TIME_PERIODIC | TIME_CALLBACK_EVENT_SET );
		if ( NULL == this->uTimerID )
		{
			this->uTimerID = NULL;
			CloseHandle ( this->hEvent );
			this->hEvent = NULL;
			char Msg[256];
			sprintf ( Msg, "timeSetEvent failed (%d).", GetLastError());
			throw TimerException( Msg, __FILE__, __LINE__ );
		}
    }

	Timer ( UINT Delay, LPTIMECALLBACK fptc, DWORD_PTR dwUser = NULL )
	{
		this->uDelay = Delay;
		this->hEvent = NULL;
		this->uTimerID = timeSetEvent( this->uDelay, 0, 
                        (LPTIMECALLBACK)fptc, dwUser,
                        TIME_PERIODIC | TIME_CALLBACK_FUNCTION );
		if ( NULL == this->uTimerID )
		{
			this->uTimerID = NULL;
			char Msg[256];
			sprintf ( Msg, "timeSetEvent failed (%d).", GetLastError());
			throw TimerException( Msg, __FILE__, __LINE__ );
		}
	}

	~Timer()
	{
		if ( NULL != this->uTimerID ) timeKillEvent ( this->uTimerID );
		if ( NULL != this->hEvent ) CloseHandle ( this->hEvent );
	}

	void Wait() throw( TimerException )
	{
		if ( NULL == this-> hEvent ) 
		{
			throw TimerException( "Invalid Timer.", __FILE__, __LINE__ );
		}
		WaitForSingleObjectEx ( this->hEvent, INFINITE, TRUE );
	}
};

EXCEPTION(ThreadException)
class Thread
{
private:
	LPTHREAD_START_ROUTINE lpStartAddress;
	LPVOID lpParameter;
	HANDLE hThread;
	DWORD ThreadId;

public:
	Thread ( LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter = NULL )
	{
		this->lpStartAddress = lpStartAddress;
		this->lpParameter = lpParameter;
		this->hThread = NULL;
		this->ThreadId = 0;
	}

	void Run()
	{
		if ( NULL != this->hThread )
		{
			char Msg[256];
			sprintf ( Msg, "Thread Routine is running.", GetLastError());
			throw ThreadException( Msg, __FILE__, __LINE__ );
		}

		this->hThread = CreateThread ( NULL,	// default security attributes
									0,			// use default stack size  
									lpStartAddress,	// thread function 
									lpParameter,		// argument to thread function 
									0,					// use default creation flags 
									&this->ThreadId );	// returns the thread identifier
		if ( NULL == this->hThread )
		{
			char Msg[256];
			sprintf ( Msg, "CreateThread failed (%d).", GetLastError());
			throw ThreadException( Msg, __FILE__, __LINE__ );
		}
	}

	void WaitExit()
	{
		if ( NULL == this->hThread ) return;
		WaitForSingleObject ( this->hThread, INFINITE );
		CloseHandle(this->hThread);
		this->hThread = NULL;
	}

	~Thread()
	{
		if ( NULL != this->hThread )
		{
			TerminateThread ( this->hThread, 1000 );
			WaitForSingleObject ( this->hThread, INFINITE );
			CloseHandle(this->hThread);
			this->hThread = NULL;
		}
	}
};

EXCEPTION(MEventException)
class MEvent
{
private:
	HANDLE eHandle;

public:
	MEvent ( char* Name = NULL, bool AutoReset = false )
	{
		if ( false == AutoReset )
		{
			this->eHandle = CreateEvent ( NULL, // default security attributes
										TRUE, // manual-reset event
										FALSE, // initial state is not signaled
										Name );  // object name
		}
		else
		{
			this->eHandle = CreateEvent ( NULL, // default security attributes
										FALSE, // auto-reset event
										FALSE, // initial state is not signaled
										Name );  // object name
		}
		if ( this->eHandle == NULL )
		{
			char Msg[256];
			sprintf ( Msg, "CreateEvent failed (%d).", GetLastError());
			throw MEventException( Msg, __FILE__, __LINE__ );
		}
	}
	~MEvent()
	{
		if ( NULL != this->eHandle ) CloseHandle(this->eHandle);
	}
	void Set()
	{
		SetEvent ( this->eHandle );
	}
	void Reset()
	{
		ResetEvent ( this->eHandle );
	}
	void Pulse()
	{
		PulseEvent ( this->eHandle );
	}
	void Wait()
	{
		WaitForSingleObject ( this->eHandle, INFINITE );
	}

};

};

#endif
