

// [ System Includes ] //
#include <iostream>

// [ Dependencies Includes ] //
#include <sp.h>

// [ SubSystem Includes ] //
#include "tmutils.h"

// [ Project ] //
#include "INetBroker.h"
#include "spreadConnector.h"

#define S_DEBUG     0x01        //< @def DEBUG Severity level.
#define S_INFO      0x02        //< @def INFO Severity level.
#define S_NOTICE    0x04        //< @def NOTICE Severity level.
#define S_WARNING   0x08        //< @def WARNING Severity level.
#define S_ERROR     0x10        //< @def ERROR Severity level.
#define S_CRITICAL  0x20        //< @def CRITICAL Severity level.
#define S_ALERT     0x40        //< @def ALERT Severity level.
#define S_EMERGENCY 0x80        //< @def EMERGENCY Severity level.


SpreadConnector::SpreadConnector
( const std::string&                __rClientName
 , const Verbosity&                 __rLogLevel )
 :
IConnector( ConnectorType::SPREAD, __rLogLevel )
, m_client_name( __rClientName )
, m_connectionStatus( CONN_DISCONNECTED )
#if defined(_TMTRACE)
, m_logHandle(DEBUG_LOG_INIT)
#endif
{
#if defined(__VERBOSE_DEBUG)

    DEBUG_LOG_START(LOG_DEBUG);

    DEBUG_LOG_END(LOG_DEBUG);

#endif

}

SpreadConnector::~SpreadConnector()
{
#if defined(__VERBOSE_DEBUG)

    DEBUG_LOG_START(LOG_DEBUG );

    DEBUG_LOG_END(LOG_DEBUG);

#endif

}




int SpreadConnector::connectNetwork(const std::string& __rAddress )
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    char connection_name[CONN_MAX_GROUP_NAME] = { '\0' };
    int mailbox = getSpreadMbox();

    int connStatus = SP_connect( __rAddress.c_str() , m_client_name.c_str(), 0, 1, &mailbox, connection_name );

    if( connStatus == CONN_JOIN_SUCCESS )
    {
        LOG_DEBUG("\nSpreadConnector: Client %s successfully connected to Server: %s"
            , m_client_name.c_str(), __rAddress.c_str() );

        setSpreadMbox(mailbox);
        setConnStatus( connStatus );
        setDaemonResource( __rAddress );
        setConnectionName( connection_name );
    }
    else
    {
        switch( connStatus )
        {
        case ILLEGAL_SPREAD:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: The spread_name given to connect to is illegal for some reason. Code: %d", connStatus);
            break;
        case COULD_NOT_CONNECT:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: Lower level socket calls failed. Code: %d", connStatus);
            break;
        case CONNECTION_CLOSED:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: During communication to establish the connection errors occurred. Code: %d", connStatus);
            break;
        case REJECT_VERSION:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: The daemon or library has a version mismatch. Code %d", connStatus);
            break;
        case REJECT_NO_NAME:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: Length of the user name was not sent.Code %d", connStatus);
            break;
        case REJECT_ILLEGAL_NAME:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: Name provided violated some requirement (length is &d or used an illegal character).Code %d", m_client_name.length(), connStatus);
            break;
        case REJECT_NOT_UNIQUE:
            DEBUG_LOG(LOG_NOTICE, "\nSpreadConnector: Name provided \'%s\' is NOT UNIQUE (replicated) on this daemon.Code %d", m_client_name.c_str(), connStatus);
            break;
        default:
            LOG_DEBUG( "\nSpreadConnector: Unknown error");
            break;
        };
    }

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif

    return connStatus;
}


int SpreadConnector::leaveNetwork()
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    if( getConnStatus() < 0 )
    {
        DEBUG_LOG( LOG_WARNING, "\nSpreadConnector: Connection was not established. Previous error code: %d", getConnStatus() );

#if defined(__VERBOSE_DEBUG)
        DEBUG_LOG_END(LOG_DEBUG);
#endif

        return getConnStatus();
    }

    int ret = SP_disconnect( getSpreadMbox() );

    if( ret < 0 )
        DEBUG_LOG( LOG_ERROR, "\nSpreadConnector: Error disconnecting: error %d", getConnStatus() );

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif
    return ret;

}


bool SpreadConnector::isConnected()
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    if( getConnStatus() < 0 )
    {
        DEBUG_LOG( LOG_WARNING, "\nSpreadConnector: Not connected! connection status: %d", getConnStatus() );

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

int SpreadConnector::joinGroup( const std::string& __rGroup )
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    int ret = SP_join( getSpreadMbox() , __rGroup.c_str() );

    if( ret != 0 )
    {
        DEBUG_LOG( LOG_ERROR, "\nSpreadConnector: Could not join group %s. error %d", __rGroup.c_str(), getConnStatus() );
    }

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif
    return ret;

}

int SpreadConnector::leaveGroup( const std::string& __rGroup )
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    int ret = SP_leave( getSpreadMbox(), __rGroup.c_str() );

    if( ret < 0 )
        DEBUG_LOG( LOG_ERROR, "\nSpreadConnector: Could not leave group %s. error %d", __rGroup.c_str(), getConnStatus() );

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif

    return ret;

}



int SpreadConnector::receiveFromGroup
( Buffer&                   __rBuffer
 , const bool               __drop_echo_messages
 , const bool               __drop_membership_messages
 )
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    char _target_group_array_buffer[MAX_GROUP_ARRAY_SIZE][MAX_GROUP_NAME] = { '\0' };
    int  _target_group_array_size = 0;
    int _endian_mismatch = 0;
    char tmpBuf[1] = { 0 };

    service service_type = 0;
    char sender[MAX_GROUP_NAME];

    int ret = 0;
    int echoMessage = 0;

    do {

        memset( _target_group_array_buffer[0], 0x00, MAX_GROUP_NAME );
        memset( sender, 0x00, MAX_GROUP_NAME );
        _target_group_array_size = 0;
        _endian_mismatch = 0;
        service_type = 0;

        __rBuffer.erase();

        // first check size => ret == size //
        ret = SP_receive
            ( m_spreadMbox
            , &service_type
            , sender
            , MAX_GROUP_ARRAY_SIZE
            , &_target_group_array_size
            , _target_group_array_buffer
            , &__rBuffer.m_opCode
            , &_endian_mismatch
            , 0
            , tmpBuf
            );

        __rBuffer.m_size = -_endian_mismatch;
        __rBuffer.m_data = (char*) calloc( __rBuffer.m_size , sizeof(char) );

        ret = SP_receive
            ( m_spreadMbox
            , &service_type
            , sender
            , MAX_GROUP_ARRAY_SIZE
            , &_target_group_array_size
            , _target_group_array_buffer
            , &__rBuffer.m_opCode
            , &_endian_mismatch
            , __rBuffer.m_size
            , __rBuffer.m_data
            );

        __rBuffer.m_groupName = std::string(_target_group_array_buffer[0] );
        __rBuffer.m_sender = std::string(sender);

        if( ( ret < 0 ) &&  (ret != BUFFER_TOO_SHORT ) )
        {

            DEBUG_LOG( LOG_ERROR, "\nSpreadConnector: Listen returned error %d", ret );

#if defined(__VERBOSE_DEBUG)
            DEBUG_LOG_END(LOG_DEBUG);
#endif

            return ret;
        }

        if( Is_membership_mess( service_type ) )
        {
            if( __drop_membership_messages == false )
            {

                DEBUG_LOG(LOG_DEBUG, "\nSpreadConnector: Receiving a MEMBERSHIP message from %s about %s"
                    , sender
                    , _target_group_array_buffer[0] );

#if defined(__VERBOSE_DEBUG)
                DEBUG_LOG_END(LOG_DEBUG);
#endif

                return CONN_MEMBERSHIP_MESSAGE;
            }
            else
                continue;
        }


        echoMessage = strcmp(getConnectionName(), sender );

        if( ( __rBuffer.m_opCode == SIG_HALT_MSG) && (echoMessage == 0 ) )
        {
            if( __rBuffer.m_groupName.empty() )
                continue;

            DEBUG_LOG( LOG_DEBUG, "\nSpreadConnector: Client %s received SIG_HALT_MSG", m_client_name.c_str() );

#if defined(__VERBOSE_DEBUG)
            DEBUG_LOG_END(LOG_DEBUG);
#endif
            return 1;
        }

        if( (echoMessage == 0 ) && ( __drop_echo_messages == false ) )
        {

            DEBUG_LOG( LOG_DEBUG, "\nSpreadConnector: Receiving a ECHO message from self %s", sender);

#if defined(__VERBOSE_DEBUG)
            DEBUG_LOG_END(LOG_DEBUG);
#endif

            return CONN_ECHO_MESSAGE;
        }

    } while( echoMessage == 0 );

    DEBUG_LOG( LOG_DEBUG, "\nSpreadConnector: Client %s received a message from group %s, Server: %s"
        , m_client_name.c_str()
        , __rBuffer.m_groupName.c_str()
        , getDaemonResource().c_str() );

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif
    return ret;

}



int SpreadConnector::sendToGroup( const Buffer& __rBuffer )
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    char _target_group_buffer[MAX_GROUP_NAME] = { '\0' };
    int  ret = 0;

    if( __rBuffer.m_groupName.size() > MAX_GROUP_NAME )
        return WRONG_GROUP_NAME_SIZE;

    memset( _target_group_buffer, 0x00, MAX_GROUP_NAME);
    memcpy(_target_group_buffer, __rBuffer.m_groupName.c_str(), __rBuffer.m_groupName.size() +1 );

    ret = SP_multicast
        ( getSpreadMbox()
        , SAFE_MESS
        , _target_group_buffer
        , __rBuffer.m_opCode
        , __rBuffer.m_size
        , __rBuffer.m_data
        );

    if( ret < 0 )
        DEBUG_LOG(LOG_ERROR, "\nSpreadConnector: Error sending to %s. error %d", _target_group_buffer, ret );

    DEBUG_LOG(LOG_DEBUG, "\nSpreadConnector: Message successfully sent by client %s on group %s"
        , m_client_name.c_str()
        , _target_group_buffer );

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif

    return ret;

}


int SpreadConnector::disconnectNetwork()
{
#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    int retVal = CONN_DISCONNECTED;

    if( getConnStatus() == CONN_JOIN_SUCCESS )
        retVal = SP_disconnect(getSpreadMbox() );


    if( retVal < 0 )
        DEBUG_LOG(LOG_ERROR,"\nSpreadConnector: Error disconnecting: error %d ", retVal );

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif
    return retVal;

}



IConnector*  WINAPI GetConnector
( const std::string&        __rClientName
 , const Verbosity&         __rLogLevel
 , const ConnectorType&     __rConnType
 )
{

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_START(LOG_DEBUG );
#endif

    switch( __rConnType() )
    {


    case ConnectorType::SPREAD:
        {
            return new SpreadConnector(__rClientName, __rLogLevel );
            break;
        }
    }

#if defined(__VERBOSE_DEBUG)
    DEBUG_LOG_END(LOG_DEBUG);
#endif

    return 0;

}

