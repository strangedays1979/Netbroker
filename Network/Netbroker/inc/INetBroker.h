


//=====================================================================================================//
/**
* @file         INetbroker.h
* @brief        Component's header interface.
* @author       Filippo Spina
* @version      0.99
* @date	        24/05/2011
*/
//=====================================================================================================//



#pragma once

// [ STL Includes ]
#include <string>

// [ System Includes ]
#include <windows.h>


#define BOOST_DISABLE_ASSERTS


#if defined(_TMTRACE)

#define LOG_DEBUG   TM_TRACE
#define LOG_INFO    TM_TRACE
#define LOG_ERROR   TM_ERROR
#define LOG_WARNING TM_WARNING

#define DEBUG_LOG_INIT         \
    ( LogInit("main", DBG_HIGH, __rDisplayLog, __rLog, SINGLE_NO, MAX_LOG_FILE) )

#define DEBUG_LOG(_logLevel, __rMessage, ... )   \
    (hTrace( _logLevel,m_logHandle,__rMessage, ##__VA_ARGS__ ) )

#define DEBUG_LOG_START(_logLevel) \
    (hTraceStart(m_logHandle, _logLevel ) )

#define DEBUG_LOG_END(_logLevel) \
    (hTraceEnd(m_logHandle, _logLevel ) )

#elif defined(_LOGSERVICE)

#pragma comment(lib, "Log_Service_vc9_mdd.lib")

#include <Log_Service.h>


#define DEBUG_LOG(_logLevel,...) _logLevel(##__VA_ARGS__)

#define DEBUG_LOG_START(...) \
    LOG_DEBUG("\n [ENTERING function %s]", __FUNCTION__)

#define DEBUG_LOG_END(...) \
    LOG_DEBUG("\n [LEAVING function %s]", __FUNCTION__)


#else
#pragma warning(disable: 4390 )

#define LOG_INFO
#define LOG_ERROR
#define LOG_WARNING

#define DEBUG_LOG_INIT 0
#define DEBUG_LOG( ... )
#define DEBUG_LOG_START( ... )
#define DEBUG_LOG_END( ... )

#endif



#pragma warning(disable: 4290)


#ifndef __NETBROKER_EXPORTDLL
#define EXPORT_NETBROKER __declspec(dllimport)
#else
#define EXPORT_NETBROKER __declspec(dllexport)
#endif

#define DISPATCHER_STOP_TIMEOUT         2000
#define SIG_HALT_MSG                    1212  // message SIG_HALT opcode ( reserved )

class Buffer;
class ConnectorType;
class Verbosity;


//====================================================================================//
/**
* @class    DispatcherException INetBroker.h "inc/INetBroker.h"
* @brief    Dispatcher exception.
* 
* @details  Dispatcher exception object.
* \n */
class EXPORT_NETBROKER DispatcherException
{

public:

    /**
    * @details      DispatcherExcepion CTOR.
    * @param[in]    __rMessage      Exception message.
    * @param[in]    __rFile         Filename.
    * @param[in]    __rLine         Line of file.
    * \n */
    DispatcherException(const std::string& __rMessage, const std::string& __rFile, const unsigned long& __rLine );

    /**
    * @details      Gets Exception message.
    * @return       Returns exception message.
    * \n */
    std::string getMsg() const;


private:
    const std::string       m_message;              //! Exception message.
    const std::string       m_file;                 //! Filename.
    const unsigned long     m_line;                 //! Line of file.

};


//====================================================================================//
/**
* @class    IDispatcher INetBroker.h "inc/INetBroker.h"
* @brief    Thread-safe, multithread interface to network comunications.
* 
* @details  Exposes a multithread, thread-safe interface to lower level comunication.
*           This layer guarantees error-handling mechanism for safe network comunications.
* \n */
class EXPORT_NETBROKER IDispatcher
{
public:

    /**
    * @details          Virtual DTOR
    * \n */
    virtual ~IDispatcher() { }

    /**
    * @details  connects to broker-server
    * @throw    DispatcherException Throws if connection fails critically.
    * \n */
    virtual bool connect()  throw( DispatcherException ) = 0;

    /**
    * @details      Registers to a Dispatcher's group.
    * @param[in]    __rGroupName        Group's name.
    * @param[in]    __rBufferSize       Group's buffer max-size.
    * @param[in]    __rQueueSize        Group's FIFO queue max-size.
    * @param[in]    __sendOnly          SEND-ONLY group!
    * @throw        DispatcherException Throw if registration fails critically.
    * \n */
    virtual void registerGroup
        ( const std::string&            __rGroupName
        , const long&                   __rBufferSize
        , const unsigned long&          __rQueueSize
        , const bool                    __sendOnly = false
        ) throw( DispatcherException ) = 0;

    /**
    * @details      Unregisters a previousl-registered group.
    * @throw        DispatcherException Throws if a critical error have been met.
    * \n */
    virtual void unregisterGroup( const std::string& __rGroupName )
        throw( DispatcherException ) = 0;

    /**
    * @details      Injects Group-made buffer into network
    * @param[in]    __pBuf      Group-made pointer to buffer.
    * @return       Returns a positive integer if message has been sent.
    * \n */
    virtual int sendMessageToGroup( const Buffer* const __pBuf ) = 0;

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
    virtual Buffer* makeMessage
        ( const std::string&            __rGroupName
        , const unsigned long&          __rSize
        , const char* const             __rData
        , const short&                  __rOpCode
        , const std::string __rCallerFuncName = ""
        ) throw( DispatcherException ) = 0;

    /**
    * @brief        Awaits a message from a given group.
    * @details      This method yelds the most requirements guarantees in terms of thread-safety
    *                   concurrent-access to groups messages. It provides a thread-safe access to each group.
    *
    * @param[in]    __rGroupName    Name of the group to which stand-by for incoming frames.
    * @param[in]    
    * @return     Returns true if operation was successfull.
    * @throw      DispatcherException Throws if operation fails critically.
    * n */
    virtual bool receiveMessageFromGroup( const std::string& __rGroupName
        , Buffer& __rBuffer
        , bool __rIsLocking = false ) throw ( DispatcherException ) = 0;

    /**
    * @details    Starts Distpacher switching its state to ON-LINE mode.
    * @throw      Throws a DispatcherException if operation fails critically.
    * \n */
    virtual void start()  throw ( DispatcherException ) = 0;

    /**
    * @details    Stops Distpacher switching its state to OFF-LINE mode.
    * @throw      DispatcherException Throws if operation fails critically.
    * \n */
    virtual bool stop()  throw( DispatcherException) = 0;

    /**
    * @details    Checks if Dispatcher is ON-LINE
    * \n */
    virtual bool isOnline() = 0;

    /**
    * @details      Disconnects from network.
    * @throw        DispatcherException Throws if operation fails critically.
    * \n */
    virtual void disconnect()  throw ( DispatcherException ) = 0;

    /**
    * @details      checks if a group has been registered.
    * @param[in]    __rGroup Group to search for.
    * @return       Returns true if group has been registered.
    * \n */
    virtual bool findGroup( const std::string __rGroup ) = 0;

};


//====================================================================================//
/**
* @class    Verbosity INetBroker.h "inc/INetBroker.h"
* @details  Wrapped, type-safe Verbosity enumerator class.
* \m */
class EXPORT_NETBROKER Verbosity
{

public:

    /** 
    * @enum Level   Verbosity Level
    * \n */
    enum Level { LOW, MEDIUM, HIGH };


public:

    /**
    * @details      Verbosity defualt CTOR.
    * \n */
    inline
        Verbosity()
        : m_level(LOW)
    { }

    /**
    * @details      Verbosity CTOR.
    * /n */
    inline 
        Verbosity( const Level& __rLevel)
        : m_level( __rLevel )
    {

    }

    /**
    * @details      Verbosity funtor.
    * \n */
    inline
        Level operator()() const
    {
        return m_level;
    }

private:
    Level m_level;              //! Verbosity level.

};


//====================================================================================//
/**
* @class    ConnectorType INetBroker.h "inc/INetBroker.h"
* @details  Wrapped, type-safe connector type enumerator class.
* \m */
class EXPORT_NETBROKER ConnectorType
{

public:

    /** 
    * @enum Type   Connector technology type.
    * \n */
    enum Type{ SPREAD };

    /**
    * @details      ConnectorType CTOR.
    * \n */
    ConnectorType( const Type& __rType )
        : m_type( __rType )
    {
    }

    /**
    * @details      ConnectorType funtor.
    * \n */
    inline
        Type operator()() const
    {
        return m_type;
    }

private:
    Type m_type;            //! Connector Type.

};


//====================================================================================//
/**
* @class    IConnector INetBroker.h "inc/INetBroker.h"
* @details  A IConnector Object handles a single network connection.
*               Each single network connection can handle different Groups.
* \m */
class EXPORT_NETBROKER IConnector
{

public:

    /**
    * @details      IConnector constructor
    * @param[in]    __rType     Connector Type.
    * @param[in]    __rLevel    Verbosity Level.
    * \n */
    inline
        IConnector( const ConnectorType& __rType, const Verbosity& __rLevel )
        : m_type( __rType )
        , m_logLevel( __rLevel )
    {

    }

    virtual ~IConnector() { }            //! Virtual DTOR.

    /**
    * @details      Creates a connection to network
    * @param[in]    __rAddress      Broker-server address.
    * @return       Returns a positive int if connection was successfull.
    * \n */
    virtual int connectNetwork(const std::string& __rAddress ) = 0;

    /**
    * @details      This method destroys a connection to an existing network.
    * @return       Returns a positive int if connection was successfull.
    * \n */
    virtual int leaveNetwork() = 0;

    /**
    * @details      This method returns connection status.
    * @return       Returns true if Connector is connected.
    * \n */
    virtual bool isConnected() = 0;

    /**
    * @details      This method joins an existing Group.
    * @param[in]    __rGroup    Technology-dependent group to join with.
    * @return       Returns a positive integer if join process succeeded.
    * \n */
    virtual int joinGroup( const std::string& __rGroup ) = 0;

    /**
    * @details      This method leaves an existing Group.
    * @param[in]    __rGroup    Registered group to leave.
    * @return       Returns a positive integer if operation was successfull.
    * \n */
    virtual int leaveGroup( const std::string& __rGroup ) = 0;

    /**
    * @details      Receives data from network layer.
    * @param[out]   Returned buffer which has to be received.
    * @param[in]    Drops returned echo messages flag.
    * @param[in]    Drop membership messages flag.
    * \n */
    virtual int receiveFromGroup(
        Buffer&                 __rBuffer
        , const bool            __drop_echo_messages        = true
        , const bool            __drop_membership_messages  = true
        ) = 0;

    /**
    * @details      Sends over a network a group-prepared and formatted buffer.
    * @param[in]    __rBuffer   Prepared buffer ready to be sent over network.
    * @return       returns a positive integer if operation was successfull.
    * \n */
    virtual int sendToGroup( const Buffer& __rBuffer ) = 0;

    /**
    * @details      Gets client unique identificator.
    * @return       Returns client's ID string.
    * \n */
    virtual std::string getClientName() const = 0;

    /**
    * @details      Disconnects from network.
    * @return       Returns a positive integer if operation was successfull.
    * \n */
    virtual int disconnectNetwork() = 0;

    inline ConnectorType getType() const { return m_type; }     //! Returns Connector Type
    inline Verbosity getLogLevel() const { return m_logLevel; } //! Returns Log Verbosity Level.

protected:

    const ConnectorType   m_type;                                     //! Connector type.
    const Verbosity       m_logLevel;                                 //! Verbosity Level.

};



//====================================================================================//
/**
* @class    Group INetBroker.h "inc/INetBroker.h"
* @brief    Group's definition
*
* @details  A Group holds information about a network broadcast channel attributes such as
*           group's size, group's unique name. It also helps construction of buffers which
*           are built by group itself in order to keep network data specification consistent.
*
* \n */
class EXPORT_NETBROKER Group
{
public:

    //====================================================================================//
    /**
    * @details      Group's constructor.
    * @param[in]    __rName             Group's name.
    * @param[in]    __rMaxBufferSize    Group's max buffer size.
    * @param[in]    __rQSize            Group's max receiving FIFO queue size.
    * \n */
    Group( const std::string& __rName, const long& __rMaxBufferSize, const long& __rQSize );

    /**
    * @details      Group's copy CTOR.
    * \n */
    Group( const Group& __ref);

    /**
    * @details      Comparison operator.
    * \n */
    inline
        bool operator==(const Group& __right) const;

    /**
    * @details      Comparison operator.
    * \n */
    inline
        bool operator<(const Group& __right ) const;

    /**
    * @details      Assignment operator.
    * \n */
    Group& operator=(const Group& __right);

    /**
    * @details      Creates a formatted buffer upon group's specifications.
    * @param[in]    __rSize     Buffer size.
    * @param[in]    __pData     Pointer to data.
    * @param[in]    __rOpCode   Frame's op-code.
    * @param[out]   __rInto     Output buffer to be generated.
    * @return       Returns true if operation was successfull.
    * \n */
    bool createBuffer
        ( const unsigned long&          __rSize
        , const char* const             __pData
        , const short&                  __rOpCode
        , Buffer&                       __rInto
        );

    /**
    * @details      Gets group's name.
    * @return       Returns group's name.
    * \n*/
    inline
        std::string getName() const;

    /**
    * @details      Gets group's buffer max size.
    * @return       Returns group's buffer max size.
    * \n*/
    inline
        long getMaxSize() const ;

    /**
    * @details      Gets group's max queue size.
    * @return       Returns group's max queue size.
    * \n*/
    inline
        unsigned long getQueueSize() const;

private:

    /**
    * @details      Sets group's name.
    * @param[in]    Group's name to be set.
    * \n */
    inline
        void setName(const std::string& __rName );

    /**
    * @details      Sets group's max buffer size spec.
    * @param[in]    __rSize Group's buffer max size to be set.
    * \n */
    inline
        void setMaxSize(const long& __rSize );

private:
    std::string         m_name;             //! Group's name
    unsigned long       m_maxBufferSize;    //! Group's buffer max size.
    unsigned long       m_queueSize;        //! Group's max incoming FIFO queue size.


};

inline
std::string Group::getName() const
{
    return m_name;
}


inline
long Group::getMaxSize() const
{

    return m_maxBufferSize;

}

inline
unsigned long Group::getQueueSize() const
{
    return m_queueSize;
}

inline
void Group::setName(const std::string& _name)
{
    m_name = _name;
}

inline
void Group::setMaxSize(const long& __rSize )
{
    m_maxBufferSize = __rSize;
}

inline
bool Group::operator==(const Group& __right) const
{
    return m_name == __right.m_name;
}

inline
bool Group::operator<(const Group& __right ) const
{
    return m_name < __right.m_name;
}



//====================================================================================//
/**
* @class    Buffer INetBroker.h "inc/INetBroker.h"
* @brief    Buffer definition
*
* @details  A Buffer is buit upon Group's specification in order to offer to upper architectural layer
*               data cohesion and specification coherence.
*
* \n */
class EXPORT_NETBROKER Buffer
{
    friend class SpreadConnector;
    friend class Group;

public:
    /**
    * @enum     MsgType     Message type.
    * \n */
    enum MsgType{ DEFAULT, REGULAR, MEMBERSHIP };

    Buffer();                                           //! Default CTOR.

    /**
    * @details      Buffer CTOR.
    * @param[in]    __rSize         Size of buffer's payload to be generated.
    * @param[in]    __rData         Pointer to raw data buffer.
    * @param[in]    __rGroupName    Group's ID association.
    * @param[in]    __rOpCode       Buffer payload frame opcode.
    * \n */
    Buffer
        ( const unsigned long&      __rSize
        , const char* const         __rData 
        , const std::string&        __rGroupName
        , const short&              __rOpCode
        );

    /**
    * @details      Buffer copy CTOR.
    * \n */
    Buffer(const Buffer& __ref);


    /**
    * @details      Buffer's assignment operator.
    * \n */
    Buffer& operator=(const Buffer& __right);


    /**
    * @details      Gets pointer to Buffer's raw data.
    * @return       Returns Buffer's pointer to raw data.
    * \n */
    inline
        char* const getPtr() const ;

    /**
    * @details      Gets pointer Buffer's opcode.
    * @return       Returns Buffer's opcode.
    * \n */
    inline
        short getOpCode() const ;

    /**
    * @details      Gets Message Type.
    * @return       Returns Message Type.
    * \n */
    inline
        MsgType getType() const ;

    inline
        std::string getGroupName() const;

    /**
    * @details      Gets size of Buffer.
    * @return       Returns size of Buffer.
    * \n */
    inline
        unsigned long getSize() const;

    /**
    * @details      Buffer DTOR.
    * \n */
    ~Buffer();

private:

    /**
    * @details      Erases buffer.
    * \n */
    void erase();

private:
    unsigned long       m_size;             //! Size of raw buffer.
    char*               m_data;             //! Pointer to raw data.
    short               m_opCode;           //! Op-code.
    std::string         m_groupName;        //! Associated group's name.
    std::string         m_sender;           //! Sender's ID.
    MsgType             m_type;             //! Message Type.

};

inline
char* const Buffer::getPtr() const
{
    return (char* const ) m_data;
}

inline
std::string Buffer::getGroupName() const
{
    return m_groupName;
}


inline
unsigned long Buffer::getSize() const
{
    return m_size;
}

inline
short Buffer::getOpCode() const
{
    return m_opCode;
}

inline
Buffer::MsgType Buffer::getType() const
{
    return m_type;
}

//====================================================================================//

/**
* @details      IDispatcher factory method.
* @param[in]    __rClientName               Client's unique ID
* @param[in]    __rAddress                  Network broker's address.
* @param[in]    __rConnType                 Connector type.
* @param[in]    __rLog                      Verbosity level.
* @param[in]    __rDropEchoMessage          Drop echo messages switch.
* @param[in]    __rDropMembershipMessages   Drop membership message switch.
*
* @return       Returns a valid IDispatcher if operation was successfull.
* \n */
EXPORT_NETBROKER IDispatcher*  WINAPI GetDispatcher
( const std::string&        __rClientName
 , const std::string&       __rAddress
 , const ConnectorType&     __rConnType
 , const Verbosity&         __rLog
 , const bool&              __rDropEchoMessage
 , const bool&              __rDropMembershipMessages
 );



/**
* @details      IConnector factory method.
* @param[in]    __rClientName               Client's unique ID.
* @param[in]    __rLogLevel                 Log verbosity level.
* @param[in]    __rConnType                 Connector type.
*   
* @return       Returns a valid IConnector pointer if operation was successfull.   
* \n */
EXPORT_NETBROKER IConnector*  WINAPI GetConnector
( const std::string&        __rClientName
 , const Verbosity&         __rLogLevel
 , const ConnectorType&     __rConnType
 );


 //====================================================================================//

