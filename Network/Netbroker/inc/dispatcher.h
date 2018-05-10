


//=====================================================================================================//
/**
* @file         disptacher.h
* @brief        Thread-safe dispatcher definitions,
* @author       Filippo Spina
* @version      0.99
* @date	        24/05/2011
*/
//=====================================================================================================//


#pragma once

#if defined(_LOGSERVICE)
#include <Log_Service.h>
#endif

// [ System Includes ]
#include <string>
#include <map>

// [ Boost Includes ]
#include <boost/shared_ptr.hpp>

// [ User Includes ]
#include <syncFifoQueue.h>

#include "INetBroker.h"

#define DISPATCHER_CTRL_GRP     "_hltd"         //! @def Dispatcher ctrl protocol

namespace boost
{
    class thread;
    class mutex;
}


typedef boost::shared_ptr<SyncFifoQueue<Buffer> >   SyncFifoBufferRef;
typedef std::map<Group, SyncFifoBufferRef>          GroupsBufferMap;
typedef boost::shared_ptr<Buffer>                   BufferRef;
typedef std::vector<Group>	                        SendOnlyGroups;


//====================================================================================//
/**
* @class    Dispatcher dispatcher.h "inc/dispatcher.h"
* @brief    Thread-safe, multithread interface to network comunications.
* 
* @details  Exposes a multithread, thread-safe interface to lower level comunication.
*           This layer guarantees error-handling mechanism for safe network comunications.
* \n*/
class Dispatcher: public IDispatcher
{

    /**
    * @enum     Status      Current connection/activity status enumerator.
    * \n */
    enum Status { OFFLINE, ONLINE, STARTED, STOPPED };

public:

    //====================================================================================//
    /**
    * @brief        Dispatcher Constructor.
    * @param[in]    __rClientName               Client ID unique network identifier.
    * @param[in]    __rAddress                  Network broker address.
    * @param[in]    __rConnType                 Connector type ( identifies which technology to be used )
    * @param[in]    __rLog                      Log level.  
    * @param[in]    __dropEchoMessage           Drop frames ECHO messages ( messages replied on the same
    *                                               registered group.
    * @param[in]    __dropMembershipMessages    Drop Membership messages ( group log-in messages )
    * @throw        DispatcherException	        Throws a Dispatcher's critical exception.
    * \n */
    Dispatcher
        ( const std::string&            __rClientName
        , const std::string&            __rAddress
        , const ConnectorType&          __rConnType
        , const Verbosity&              __rLog
        , const bool                    __dropEchoMessage = false
        , const bool                    __dropMembershipMessages  = true
        ) throw( DispatcherException );


    /**
    * @details          Copy CTOR
    * \n */
    Dispatcher(const Dispatcher& __ref) throw( DispatcherException );


    /**
    * @details          Virtual DTOR
    * \n */
    virtual ~Dispatcher();


    /**
    * @details Assignment operator.
    * \n */
    Dispatcher& operator=(const Dispatcher& __ref ) throw( DispatcherException );

    /**
    * @details      Change default server-address
    * @param[in]    __rAddress      Server address string.
    * \n */
    void changeDefaultAddress(const std::string& __rAddress);

    /**
    * @details  connects to broker-server
    * @throw    DispatcherException Throws if connection fails critically.
    * \n */
    bool connect()  throw( DispatcherException );

    /**
    * @details      Registers to a Dispatcher's group.
    * @param[in]    __rGroupName        Group's name.
    * @param[in]    __rBufferSize       Group's buffer max-size.
    * @param[in]    __rQueueSize        Group's FIFO queue max-size.
    * @param[in]    __sendOnly          SEND-ONLY group!
    * @throw        DispatcherException Throw if registration fails critically.
    * \n */
    void registerGroup
        ( const std::string&            __rGroupName
        , const long&                   __rBufferSize
        , const unsigned long&          __rQueueSize
        , const bool                    __sendOnly = false
        ) throw( DispatcherException );

    /**
    * @details      Unregisters a previousl-registered group.
    * @throw        DispatcherException Throws if a critical error have been met.
    * \n */
    void unregisterGroup( const std::string& __rGroupName )
        throw( DispatcherException );

    /**
    * @brief        Awaits a message from a given group.
    * @details      This method yelds the most requirements guarantees in terms of thread-safety
    *                   concurrent-access to groups messages. It provides a thread-safe access to each group.
    *
    * @param[in]    __rGroupName    Name of the group to which stand-by for incoming frames.
    * @param[out]   __rBuffer       Returned buffer.
    * @param[in]    __isLocking     Operation is thread-blocking.
    *
    * @return     Returns true if operation was successfull.
    * @throw      DispatcherException Throws if operation fails critically.
    * n */
    bool receiveMessageFromGroup
        ( const std::string&            __rGroupName
        , Buffer&                       __rBuffer
        , bool                          __isLocking = false
        ) throw ( DispatcherException );

    /**
    * @details    Starts Distpacher switching its state to ON-LINE mode.
    * @throw      Throws a DispatcherException if operation fails critically.
    * \n */
    void start()  throw ( DispatcherException );

    /**
    * @details    Stops Distpacher switching its state to OFF-LINE mode.
    * @throw      DispatcherException Throws if operation fails critically.
    * \n */
    bool stop()  throw( DispatcherException);

    /**
    * @details    Checks if Dispatcher is ON-LINE
    * \n */
    bool isOnline();

    /**
    * @details      Disconnects from network.
    * @throw        DispatcherException Throws if operation fails critically.
    * \n */
    void disconnect()  throw ( DispatcherException );

    /**
    * @details      Gets current Status.
    * @return       Returns current Status.
    * \n */
    inline
        Status getActivityStatus() const;


    /**
    * @details      checks if a group has been registered.
    * @param[in]    __rGroup Group to search for.
    * @return       Returns true if group has been registered.
    * \n */
    bool findGroup( const std::string __rGroup );


    /**
    * @details      Injects Group-made buffer into network
    * @param[in]    __pBuf      Group-made pointer to buffer.
    * @return       Returns a positive integer if message has been sent.
    * \n */
    int sendMessageToGroup( const Buffer* const __pBuf );

    /**
    * @details      Prepares a buffer upon group specifications to be injected in network.
    * @param[in]    __rGroupName        Name of the destination group to which format data.
    * @param[in]    __rSize             Size of the buffer to be sent.
    * @param[in]    __pData             Pointer to data buffer to be sent.
    * @param[in]    __rOpCode           User OpCode.
    * @param[in]    __rCallerFuncName   Caller function name ( hidden not to be used by user ).
    * @return       Buffer              Returns pointer to formatted buffer.
    * @throw        DispatcherException Throws a critical exception if buffer specifications are not met.
    * \n */
    Buffer* makeMessage
        ( const std::string&            __rGroupName
        , const unsigned long&          __rSize
        , const char* const             __pData
        , const short&                  __rOpCode
        , const std::string             __rCallerFuncName = ""
        ) throw( DispatcherException );

private:
    /**
    * @details      Sets current Status.
    * @param[in]    __rActivity Status to be set.
    * \n */
    inline
        void setActivityStatus(const Status __rActivity );

    /**
    * @details  Thread-related job
    * \n */
    void job();

    /**
    * @details  Notifies about new instance creation.
    * \n */
    void notifyNewInstance();
    /**
    * @details  Notifies about instance deletion.
    * \n */
    void notifyDeleteInstance();

private:
    Status                      m_connStatus;               //! Dispatcher current connection status.
    Status                      m_activity;                 //! Current activity status.
    mutable boost::mutex        m_activityMutex;            //! Thread-safe activity mutex.

    boost::thread*              m_pThread;                  //! Thread-associated mutex.
    GroupsBufferMap             m_group_buffer_map;         //! Current registered SEND/RECEIVE groups.

    SendOnlyGroups	            m_sendOnlyGroups;           //! SEND-ONLY groups.

    IConnector*                 m_connector;                //! Selected network Connector.
    std::string                 m_address;                  //! Broker server address.

    unsigned long               m_instances;                //! Number of instances.
    Dispatcher*                 m_pParentCpy;               //! Pointer to parent dispatcher.

    const bool                  m_dropEchoMessages;         //! Drop echo messages flag.
    const bool                  m_dropMembershipMessages;   //! Drop membership messages flag.

    boost::recursive_mutex      m_dispatcherMutex;          //! Thread-safe Dispatcher mutex.

    ConnectorType               m_connType;                 //! Connector Type

};


inline
Dispatcher::Status Dispatcher::getActivityStatus() const
{
    boost::unique_lock<boost::mutex> lock(m_activityMutex);
    return m_activity;
}


inline
void Dispatcher::setActivityStatus(const Dispatcher::Status _activity)
{
    boost::unique_lock<boost::mutex> lock(m_activityMutex);
    m_activity = _activity;
}



//=====================================================================================================//

