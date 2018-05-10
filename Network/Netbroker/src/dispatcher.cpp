

// [ System Includes ] //
#include <sstream>

// [ SPREAD Includes ] //
#include <sp.h>

// [ User Includes ] //
#include "Dispatcher.h"
#include "SpreadConnector.h"


IDispatcher* WINAPI GetDispatcher
(  const std::string&         __rClientName
 , const std::string&         __rAddress
 , const ConnectorType&       __rConnType
 , const Verbosity&           __rLog
 , const bool&                __rDropEchoMessage
 , const bool&                __rDropMembershipMessages
 )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif


	return new Dispatcher( __rClientName, __rAddress, __rConnType, __rLog, __rDropEchoMessage, __rDropMembershipMessages );

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG );
#endif
}

DispatcherException::DispatcherException
( const std::string& __rMessage
 , const std::string& __rFile
 , const unsigned long& __rLine )
 : m_message(__rMessage)
 , m_file( __rFile )
 , m_line( __rLine )
{  }

std::string DispatcherException::getMsg() const
{
	std::ostringstream oss;
	oss << "[" << m_file << ":" << m_line << "] " << m_message;
	return oss.str();
}

void Dispatcher::registerGroup(const std::string&       __rGroupName
							   , const long&            __rBufferSize
							   , const unsigned long&   __rQueueSize
							   , const bool sendOnly ) throw( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif

	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( __rBufferSize > 92160 )  /** 90 kb buffer size limit */
		throw DispatcherException("Group's buffer size exceeds Spread specifications", __FILE__, __LINE__ );

	if( __rGroupName.size() >= CONN_MAX_GROUP_NAME )
		throw DispatcherException("Group name size exceeded!", __FILE__, __LINE__ );

	Group newGroup( __rGroupName, __rBufferSize, __rQueueSize);

	GroupsBufferMap::const_iterator iter = m_group_buffer_map.find( newGroup );

	if( iter != m_group_buffer_map.end() )
		throw DispatcherException("Group " + __rGroupName + " already registered", __FILE__, __LINE__ );

	if( sendOnly )
	{
		for( register unsigned i= 0; i< m_sendOnlyGroups.size(); ++i )
		{
			if( m_sendOnlyGroups[i] == newGroup )
				throw DispatcherException("Send only group " + __rGroupName +" already registered", __FILE__, __LINE__ );
		}

		LOG_DEBUG("\nJoining Spread group (SEND-ONLY) %s", __rGroupName.c_str() );

		m_sendOnlyGroups.push_back(newGroup );
	}
	else
	{

		m_group_buffer_map.insert( 
			GroupsBufferMap::value_type( newGroup, SyncFifoBufferRef( new SyncFifoQueue<Buffer>(__rQueueSize) )));

		LOG_DEBUG("\nJoining Spread group (SEND/RECEIVE) %s", __rGroupName.c_str() );

		if( m_connStatus == ONLINE )
			m_connector->joinGroup(__rGroupName);
		else
			throw DispatcherException("Network not connected", __FILE__, __LINE__ );
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif

}


void Dispatcher::unregisterGroup( const std::string& __rGroupName ) throw( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);


	if( __rGroupName.size() >= CONN_MAX_GROUP_NAME )
		throw DispatcherException("Group name size exceeded!", __FILE__, __LINE__ );

	for( SendOnlyGroups::iterator iter = m_sendOnlyGroups.begin();
		iter != m_sendOnlyGroups.end(); ++iter )
	{
		if( (*iter).getName() == __rGroupName )
			m_sendOnlyGroups.erase(iter);

#if defined(__VERBOSE_DEBUG)
		DEBUG_LOG_END(LOG_DEBUG);
#endif
		return;
	}


	GroupsBufferMap::iterator iter = m_group_buffer_map.find( Group(__rGroupName, 0, 0) );

	if( iter != m_group_buffer_map.end() )
	{
		if( m_connStatus == ONLINE )
			m_connector->leaveGroup(iter->first.getName() );
		else
			throw DispatcherException("Network not connected", __FILE__, __LINE__ );

		m_group_buffer_map.erase(iter);

#if defined(__VERBOSE_DEBUG)
		DEBUG_LOG_END(LOG_DEBUG);
#endif
		return;
	}

	throw DispatcherException("Unable to remove group: group not found!", __FILE__, __LINE__ );

}

Buffer* Dispatcher::makeMessage
( const std::string& __rGroupName
 , const unsigned long& __rSize
 , const char* const __rData
 , const short& __rOpCode
 , const std::string __rCallerFuncName
 ) throw( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif

	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( m_connStatus == OFFLINE )
		throw DispatcherException("Network not connected", __FILE__, __LINE__ );

	if( __rCallerFuncName != "?stop@Dispatcher@@UAE_NXZ" )
		if( __rOpCode == SIG_HALT_MSG )
			throw DispatcherException( "Reserved op code!", __FILE__, __LINE__ );

	Buffer* buf = new Buffer();

	for( register unsigned i= 0; i< m_sendOnlyGroups.size(); ++i )
	{
		if( m_sendOnlyGroups[i].getName() ==  __rGroupName )
		{
			if( m_sendOnlyGroups[i].createBuffer(__rSize, __rData, __rOpCode, *buf ) == false )
				throw DispatcherException( "ERROR: Message size did not pass group " + __rGroupName + " specification" , __FILE__, __LINE__ );

#if defined(__VERBOSE_DEBUG)
			DEBUG_LOG_END(LOG_DEBUG);
#endif
			return buf;

		}
	}

	GroupsBufferMap::iterator iter = m_group_buffer_map.find(Group(__rGroupName, 0, 0));

	if( iter == m_group_buffer_map.end() )
		throw DispatcherException( "Group not registered", __FILE__, __LINE__ );


	if( ( ((Group& ) iter->first).createBuffer(__rSize, __rData, __rOpCode, *buf ) ) == false )
		throw DispatcherException( "ERROR: Message size did not pass group " + __rGroupName + " specification" , __FILE__, __LINE__ );

#if defined(__VERBOSE_DEBUG)
			DEBUG_LOG_END(LOG_DEBUG);
#endif
	return buf;
}


int Dispatcher::sendMessageToGroup( const Buffer*const  __pBuffer )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	LOG_DEBUG("\nSending message to group: %s, Server: %s"
		, __pBuffer->getGroupName().c_str()
		, m_address.c_str() );

	return m_connector->sendToGroup(*__pBuffer);

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG );
#endif

}


bool Dispatcher::receiveMessageFromGroup
( const std::string&                        __rGroupName
 , Buffer&                                   __rBuffer
 , const bool                                __isLocking
 ) throw( DispatcherException )
{

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif

	bool retVal = false;
	SyncFifoQueue<Buffer>  *_pItem = 0;

	{
		boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

		if( getActivityStatus() == STOPPED )
			return false;


		GroupsBufferMap::iterator iter = m_group_buffer_map.find( Group(__rGroupName, 0, 0) );

		if( iter == m_group_buffer_map.end() )
			throw DispatcherException("Group not found!", __FILE__, __LINE__ );

		_pItem = iter->second.get();

	}


	if( !__isLocking )
		retVal = _pItem->async_pop(__rBuffer);
	else
	{
		_pItem->pop(__rBuffer );
		retVal = true;
	}

	LOG_DEBUG("\nMessage from group: %s, Server: %s popped from queue"
		, __rGroupName.c_str()
		, m_address.c_str()  );

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return retVal;

}


Dispatcher::Dispatcher
(  const std::string&           __rClientName
 , const std::string&           __rAddress
 , const ConnectorType&         __rConnType
 , const Verbosity&             __rLog
 , const bool                   __dropEchoMessages
 , const bool                   __dropMembershipMessages ) throw( DispatcherException)
 : m_pThread(0)
 , m_connector(0)
 , m_connStatus(OFFLINE)
 , m_address( __rAddress )
 , m_instances(1)
 , m_pParentCpy(0)
 , m_activity( STOPPED )
 , m_dropEchoMessages( __dropEchoMessages )
 , m_dropMembershipMessages( __dropMembershipMessages )
 , m_connType(__rConnType)
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	switch (m_connType() )
	{
	case ConnectorType::SPREAD:
		{

			// TODO: check client name size respects spread specifications

			/*int size = __rClientName.size();

			if( size > 10 )
			throw DispatcherException( "Client name size exceeds SPREAD specifications", __FILE__, __LINE__);*/

			m_connector = new SpreadConnector(__rClientName, __rLog  );


			break;
		}
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG );
#endif
}

void Dispatcher::changeDefaultAddress(const std::string& __rAddress)
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( __rAddress != m_address )
	{
		m_address = __rAddress;
	}
}

bool Dispatcher::connect() throw( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( m_connStatus == ONLINE )
		throw DispatcherException("Dispatcher is still connected. Disconnect first.", __FILE__, __LINE__ );

	if( getActivityStatus() == STARTED )
		throw DispatcherException("Dispatcher is still running. Stop it first!", __FILE__, __LINE__ );


	if( (m_connector->connectNetwork(m_address)) >= 0 )
	{
		LOG_DEBUG("\nDispatcher is connected to server %s", m_address.c_str() );

		m_connStatus = ONLINE;
		registerGroup( m_connector->getClientName() + std::string(DISPATCHER_CTRL_GRP ) , 1, 1 );

#if defined(__VERBOSE_DEBUG)
		DEBUG_LOG_END(LOG_DEBUG);
#endif
		return true;
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return false;
}

void Dispatcher::start() throw ( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( m_connStatus == OFFLINE )
		throw DispatcherException( "Dispatcher is not connected!", __FILE__, __LINE__ );

	if( getActivityStatus() == STARTED )
		throw DispatcherException( "Dispatcher is already started. Stop it first!", __FILE__, __LINE__ );


	m_pThread = new boost::thread( boost::bind(&Dispatcher::job, this) );
	setActivityStatus(STARTED);
}

void Dispatcher::notifyNewInstance()
{
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	++m_instances;
	if( m_pParentCpy && ( m_instances != 1 ) )
		m_pParentCpy->notifyNewInstance();
}

void Dispatcher::notifyDeleteInstance()
{
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	--m_instances;
	if( m_pParentCpy && ( m_instances != 1 ) )
		m_pParentCpy->notifyDeleteInstance();
}

Dispatcher::Dispatcher(const Dispatcher& __ref) throw( DispatcherException )
: m_dropEchoMessages( __ref.m_dropEchoMessages )
, m_dropMembershipMessages( __ref.m_dropMembershipMessages )
, m_connType( __ref.m_connType )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG);
#endif

	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	m_pThread = 0;
	m_connector = 0;   // this instance will have his new connector
	m_connStatus = OFFLINE;
	setActivityStatus(STOPPED);
	m_address = __ref.m_address;
	m_pParentCpy = const_cast<Dispatcher*> (&__ref );


	switch (__ref.m_connector->getType()() )
	{
	case ConnectorType::SPREAD:
		{
			// increment counter and send notification to parent's dispatchers
			notifyNewInstance();

			// make a new client string
			std::ostringstream oss;
			oss << __ref.m_connector->getClientName() << "_cpy_" << m_instances;

			if( oss.str().size() > 10 )
				throw DispatcherException( "Client name size exceeds SPREAD specifications", __FILE__, __LINE__);

			// start subengine
			m_connector = new SpreadConnector( oss.str() 
				, __ref.m_connector->getLogLevel() );

			break;
		}
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif

}

Dispatcher& Dispatcher::operator=(const Dispatcher& __ref ) throw (DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG);
#endif

	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( getActivityStatus() == STARTED )
		stop();

	if( m_connStatus == ONLINE )
		disconnect();

	// delete this connector
	if( m_connector )
	{
		delete m_connector;
		m_connector = 0;
	}

	m_connStatus = OFFLINE;
	setActivityStatus(STOPPED);
	m_address = __ref.m_address;
	m_pParentCpy = const_cast<Dispatcher*> (&__ref );


	switch (__ref.m_connector->getType()() )
	{
	case ConnectorType::SPREAD:
		{
			// increment counter and send notification to parent's dispatchers
			notifyNewInstance();

			// make a new client string
			std::ostringstream oss;
			oss << __ref.m_connector->getClientName() << "_cpy_" << m_instances;

			if( oss.str().size() > 10 )
				throw DispatcherException( "Client name size exceeds SPREAD specifications", __FILE__, __LINE__);

			// start subengine
			m_connector = new SpreadConnector
                ( oss.str() 
				, __ref.m_connector->getLogLevel()
                );

			break;
		}
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return *this;
}


Dispatcher::~Dispatcher()
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG);
#endif

	if( getActivityStatus() == STARTED )
		stop();

	if( m_connStatus == ONLINE )
		disconnect();


	// signal recursively all actives PARENT dispatcher copies about performing deletion.
	// constraint:   DELETION order must be respected Child -> Parent
	notifyDeleteInstance();


	// delete connector
	if( m_connector )
	{
		delete m_connector;
		m_connector = 0;
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
}

bool Dispatcher::isOnline()
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG);
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	bool retVal = false;


	if( m_connStatus == ONLINE )
		retVal = true;
	else if( m_connStatus == OFFLINE )
		retVal = false;

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return retVal;
}

bool Dispatcher::stop() throw( DispatcherException)
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( m_connStatus == OFFLINE )
		throw DispatcherException( "Dispatcher is not connected!", __FILE__, __LINE__ );

	if( getActivityStatus() == STOPPED )
		throw DispatcherException( "Dispatcher has already been stopped!", __FILE__, __LINE__ );

	char exit[] = { 0x00 };

	try
	{
		sendMessageToGroup( makeMessage( m_connector->getClientName() + std::string(DISPATCHER_CTRL_GRP ) , 1, exit, SIG_HALT_MSG, __FUNCDNAME__ ) );


	}catch( DispatcherException &ex)
	{
		throw ex;
	}


	if( m_pThread )
	{
		bool ret = m_pThread->timed_join(boost::posix_time::milliseconds(DISPATCHER_STOP_TIMEOUT) );
		if( ret == true )
		{
			delete m_pThread;
			m_pThread = 0;
		} else
		{
			m_connector->disconnectNetwork();            
			delete m_pThread;
			m_pThread = 0;

			setActivityStatus(STOPPED);

#if defined(__VERBOSE_DEBUG)
			DEBUG_LOG_END(LOG_DEBUG);
#endif
			return false;
		}
	}

	setActivityStatus(STOPPED);

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return true;
}

void Dispatcher::disconnect() throw( DispatcherException )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif

	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	if( getActivityStatus() == STARTED )
		throw DispatcherException( "Dispatcher has not yet been stopped. Stop it before disconnecting", __FILE__, __LINE__);


	if( m_connStatus == ONLINE )
	{
		unregisterGroup( m_connector->getClientName() + std::string(DISPATCHER_CTRL_GRP ) );
		m_connector->disconnectNetwork();
		m_connStatus = OFFLINE;

	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
}


void Dispatcher::job()
{

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif

	Buffer retBuf;
	int    retVal;

	while( isOnline() )
	{

		retVal = m_connector->receiveFromGroup(retBuf,m_dropEchoMessages, m_dropMembershipMessages);

		if( retBuf.getOpCode() == SIG_HALT_MSG )
			if(  retBuf.getGroupName() == m_connector->getClientName() + std::string(DISPATCHER_CTRL_GRP ) )
			{
#if defined(__VERBOSE_DEBUG)
				DEBUG_LOG_END(LOG_DEBUG);
#endif
				return;
			}

			if( retVal >= 0 )
			{
				SyncFifoQueue<Buffer>* pRef = 0;

				{
					boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

					GroupsBufferMap::const_iterator iter = m_group_buffer_map.find( Group(retBuf.getGroupName(), 0, 0) );

					if( iter == m_group_buffer_map.end() )
						continue;

					if( iter->second->isFull() )
					{
						LOG_WARNING( "Group queue %s is FULL!!!! \n", retBuf.getGroupName().c_str() );
						continue;
					}

					pRef = iter->second.get();

				}

				pRef->push(retBuf);

                LOG_DEBUG(  "Data from group %s pushed in Dispatcher queue", retBuf.getGroupName().c_str() );

			}

			if( (retVal == CONNECTION_CLOSED)  || (retVal == NET_ERROR_ON_SESSION) )
			{
				LOG_WARNING(  "SPREAD server connection went offline. Shutting down connection: ");

				try
				{
					stop();
					disconnect();

				} catch( ... ) 
				{
					LOG_WARNING( "Shutdown NOT COMPLETED!.\n\n" );
				}

				LOG_INFO( "Shutdown COMPLETED!.\n\n");
				break;
			}

	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif

}



bool Dispatcher::findGroup( const std::string __rGroup )
{
#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_START(LOG_DEBUG );
#endif
	boost::unique_lock<boost::recursive_mutex> lock(m_dispatcherMutex);

	for( register unsigned i= 0; i< m_sendOnlyGroups.size(); ++i )
	{
		if( m_sendOnlyGroups[i].getName() == __rGroup )
			return true;
	}


	GroupsBufferMap::iterator iter = m_group_buffer_map.find( Group(__rGroup, 0, 0) );

	if( iter == m_group_buffer_map.end() )
	{
#if defined(__VERBOSE_DEBUG)
		DEBUG_LOG_END(LOG_DEBUG);
#endif
		return false;
	}

#if defined(__VERBOSE_DEBUG)
	DEBUG_LOG_END(LOG_DEBUG);
#endif
	return true;

}

