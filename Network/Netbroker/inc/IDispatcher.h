


#pragma once

#include "connector.h"

#include <windows.h>

#include <string>

class DispatcherException;
class Buffer;

class EXPORT_NETBROKER IDispatcher
{
public:

	virtual ~IDispatcher() { }

	virtual bool connect()  throw( DispatcherException ) = 0;

	virtual void registerGroup(
		const std::string& _groupName
		, const long& _bufferSize
		, const unsigned long& _qsize
		, const bool sendOnly = false )
		throw( DispatcherException ) = 0;

	virtual void unregisterGroup( const std::string& _groupName )
		throw( DispatcherException ) = 0;

	virtual int sendMessageToGroupManaged( const Buffer* __pBuf ) = 0;


	virtual Buffer* makeMessageManaged(
		const std::string& _groupName
		, const unsigned long& _size
		, const char* const _data
		, const short& _opCode
		, const std::string _callerFuncName = "" )
		throw( DispatcherException ) = 0;

	virtual bool receiveMessageFromGroup( const std::string& _groupName
		, Buffer& _buf
		, bool _isLocking = false ) throw ( DispatcherException ) = 0;

	virtual void start()  throw ( DispatcherException ) = 0;
	virtual bool stop()  throw( DispatcherException) = 0;
	virtual bool isOnline() = 0;

	virtual void disconnect()  throw ( DispatcherException ) = 0;

	virtual bool findGroup( const std::string _group ) = 0;



};


EXPORT_NETBROKER IDispatcher*  WINAPI GetDispatcher
(
 const std::string& _client_name
 , const std::string& _address
 , const Connector::ConnectorType _connType
 , const Connector::LogLevel _log
 , const bool _displayLog
 , const bool _dropEchoMessage
 , const bool _dropMembershipMessages
 );