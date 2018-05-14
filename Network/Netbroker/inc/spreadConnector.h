


//=====================================================================================================//
/**
* @file         spreadConnector.h
* @brief        SPREAD Technology specialized connector iterface.
* @author       Filippo Spina
* @version      0.99
* @date	        24/05/2011
*/
//=====================================================================================================//



#pragma once

#if defined(_LOGSERVICE)
#include <Log_Service.h>
#endif


// [ System Includes ] //
#include <string>

#include <boost/thread.hpp>

// [ Project includes ] // 
#include "INetBroker.h"


#define CONN_JOIN_SUCCESS 1					///< NORMAL

// [ BAD RESULTS MUST BE NEGATIVE VALUES ] //

// [ INHERITED FROM SPREAD ] //
#define CONN_ILLEGAL_DAEMON_RESOURCE    -1  //! ILLEGAL_SPREAD
#define CONN_NETWORK_FAILED             -2	//! COULD_NOT_CONNECT
#define CONN_CONNECTION_REFUSED         -8	//! CONNECTION_CLOSED
#define CONN_WRONG_VERSION              -7	//! REJECT_VERSION
#define CONN_ILLEGAL_CONNECTION_NAME    -5	//! REJECT_ILLEGAL_NAME
#define CONN_CONNECTION_ALREADY_EXISTS  -6	//! REJECT_NOT_UNIQUE				  
#define CONN_CONNECTION_ACCEPTED        1	//! ACCEPT_SESSION
#define CONN_ILLEGAL_GROUP              -14	//! ILLEGAL_GROUP
#define CONN_ILLEGAL_CONNECTION         -11	//! ILLEGAL_SESSION
#define CONN_CONNECTION_REFUSED         -8	//! CONNECTION_CLOSED


// [ USER DEFINED ]//
#define CONN_MAX_GROUP_NAME             32  //! MAX group name size.

#define CONN_MEMBERSHIP_MESSAGE     11111   //! Return value: a membership message has been received
#define CONN_ECHO_MESSAGE           111111  //! Return value: an ECHO message has been received.
#define WRONG_GROUP_NAME_SIZE       -11111  //! Group's name is exceeding maximum size allowed.
#define CONN_DISCONNECTED           -111111 //! Connection has been closed error!

#define MAX_GROUP_ARRAY_SIZE        100     //! Max number of groups allowed.


//====================================================================================//
/**
* @class    SpreadConnector INetBroker.h "inc/INetBroker.h"
* @details  A SpreadConnector implements an IConnector upon SPREAD network specifications.
*
* \m */
class SpreadConnector: public IConnector
{
public:

    /**
    * @details      IConnector constructor
    * @param[in]    __rClientName   Client's unique ID.
    * @param[in]    __rLogLevel     Verbosity Level.
    * \n */
    SpreadConnector
        ( const std::string&            __rClientName
        , const Verbosity&              __rLogLevel
        );

    virtual ~SpreadConnector();     //! Default DTOR.

    /**
    * @details      Creates a connection to network
    * @param[in]    __rAddress      Broker-server address.
    * @return       Returns a positive int if connection was successfull.
    * \n */
    int connectNetwork(const std::string& __rAddress );

    /**
    * @details      Disconnects from network.
    * @return       Returns a positive integer if operation was successfull.
    * \n */
    int disconnectNetwork();

    /**
    * @details      This method destroys a connection to an existing network.
    * @return       Returns a positive int if connection was successfull.
    * \n */
    int leaveNetwork();

    /**
    * @details      This method returns connection status.
    * @return       Returns true if Connector is connected.
    * \n */
    bool isConnected();

    /**
    * @details      This method joins an existing Group.
    * @param[in]    __rGroup    Technology-dependent group to join with.
    * @return       Returns a positive integer if join process succeeded.
    * \n */
    int joinGroup( const std::string& __rGroup );

    /**
    * @details      This method leaves an existing Group.
    * @param[in]    __rGroup    Registered group to leave.
    * @return       Returns a positive integer if operation was successfull.
    * \n */
    int leaveGroup( const std::string& __rGroup );

    /**
    * @details      Receives data from network layer.
    * @param[out]   Returned buffer which has to be received.
    * @param[in]    Drops returned echo messages flag.
    * @param[in]    Drop membership messages flag.
    * \n */
    int receiveFromGroup(
        Buffer& __rBuffer
        , const bool _drop_echo_messages 
        , const bool _drop_membership_messages);

    /**
    * @details      Sends over a network a group-prepared and formatted buffer.
    * @param[in]    __rBuffer   Prepared buffer ready to be sent over network.
    * @return       returns a positive integer if operation was successfull.
    * \n */
    int sendToGroup( const Buffer& __rBuffer );

    /**
    * @details      Gets client unique identificator.
    * @return       Returns client's ID string.
    * \n */
    inline
        std::string getClientName() const;

    /**
    * @details      Gets SPREAD server address.
    * @return       Returns a string with SPREAD server address.
    * \n */
    inline
        std::string getDaemonResource() const;

    /**
    * @details      Gets Connector' Status.
    * \n */
    inline
        int getConnStatus() const ;

    /**
    * @details      Gets SPREAD server address.
    * @return       Returns an integer resulting in connection status ( if positive network is connected )
    * \n */
    inline
        int getSpreadMbox() const;    

    /**
    * @details      Gets SPREAD Connection unique name.
    * @return       Returns a char string to current Connection Name.
    * \n */
    inline
        char*const getConnectionName() const;


private:

    /**
    * @details      Sets SPREAD server address.
    * @param[in]    __rDaemonResource       SPREAD server address.
    * \n */
    inline
        void setDaemonResource(const std::string& __rDaemonResource);

    /**
    * @details      Sets Connector' Status.
    * @param[in]    __rConnStatus           Connector' Status to be set.
    * \n */
    inline
        void setConnStatus( const int& __rConnStatus );

    /**
    * @details      Sets SPREAD server address.
    * @param[in]    __rMbox         Spread MBOX to be set.
    * \n */
    inline
        void setSpreadMbox(const int& __rMbox );

    /**
    * @details      Sets SPREAD Connection unique name.
    * @param[in]    __rpConnName     Pointer to connection ID to be set.
    * \n */
    inline
        void setConnectionName(const char __rpConnName[]);

private:
    int m_spreadMbox;                                               //! SPREAD mbox.
    const std::string       m_client_name;                          //! Client's name.
    std::string             m_spread_daemon_resource;               //! SPREAD server address.
    char                    m_connection_name[CONN_MAX_GROUP_NAME]; //! SPREAD set connection ID.
    int                     m_connectionStatus;                     //! Integer representing conn. status.
    mutable boost::mutex    m_connectorMutex;                       //! Connector mutex ( for mutual access )

#if defined(_TMTRACE)
    const int               m_logHandle;                            //! Log handle for TMTRACE only.
#endif

};


inline
std::string SpreadConnector::getClientName() const
{
    return m_client_name;
}

inline
std::string SpreadConnector::getDaemonResource() const
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    return m_spread_daemon_resource;
}

inline
void SpreadConnector::setDaemonResource(const std::string& __rDaemonResource)
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    m_spread_daemon_resource = __rDaemonResource;

}

inline
void SpreadConnector::setConnStatus( const int& __rConnStatus )
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    m_connectionStatus = __rConnStatus;

}

inline
int SpreadConnector::getConnStatus() const
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    return m_connectionStatus;

}


int SpreadConnector::getSpreadMbox() const
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    return m_spreadMbox;

}


void SpreadConnector::setSpreadMbox(const int& __rMbox )
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    m_spreadMbox = __rMbox;

}


void SpreadConnector::setConnectionName(const char __rpConnName[])
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    memcpy( m_connection_name, __rpConnName, CONN_MAX_GROUP_NAME );

}

char*const SpreadConnector::getConnectionName() const
{
    boost::unique_lock<boost::mutex> lock(m_connectorMutex);
    return (char*const) m_connection_name;

}