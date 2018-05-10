
//=====================================================================================================//
/**
* @file         Log_Service.h
* @brief        Contains Log_Service exposed interface definitions
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/
//=====================================================================================================//


#pragma once

#pragma warning(disable: 4290 )

#ifndef __EXPORTDLL
#define EXPORT __declspec(dllimport)
#else
#define EXPORT __declspec(dllexport)
#endif


/**
*       [ MACRO WRAP DEFINITIONS ]
*/
#ifndef LOG_DEBUG
#define LOG_DEBUG(...)                              \
    ILogService::getPtr()->Log_DEBUG(__VA_ARGS__);
#endif

#ifndef LOG_INFO
#define LOG_INFO(...)                              \
    ILogService::getPtr()->Log_INFO(__VA_ARGS__);
#endif

#ifndef LOG_NOTICE
#define LOG_NOTICE(...)                              \
    ILogService::getPtr()->Log_NOTICE(__VA_ARGS__);
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(...)                              \
    ILogService::getPtr()->Log_WARNING(__VA_ARGS__);
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...)                              \
    ILogService::getPtr()->Log_ERROR(__VA_ARGS__);
#endif

#ifndef LOG_CRITICAL
#define LOG_CRITICAL(...)                              \
    ILogService::getPtr()->Log_CRITICAL(__VA_ARGS__);
#endif

#ifndef LOG_ALERT
#define LOG_ALERT(...)                              \
    ILogService::getPtr()->Log_ALERT(__VA_ARGS__);
#endif

#ifndef LOG_EMERGENCY
#define LOG_EMERGENCY(...)                              \
    ILogService::getPtr()->Log_EMERGENCY(__VA_ARGS__);
#endif


/**
* [ Supported severity levels ]
*/
#ifndef S_DEBUG
#define S_DEBUG     0x01        //< @def DEBUG Severity level.
#endif

#ifndef S_INFO
#define S_INFO      0x02        //< @def INFO Severity level.
#endif

#ifndef S_NOTICE
#define S_NOTICE    0x04        //< @def NOTICE Severity level.
#endif

#ifndef S_WARNING
#define S_WARNING   0x08        //< @def WARNING Severity level.
#endif

#ifndef S_ERROR
#define S_ERROR     0x10        //< @def ERROR Severity level.
#endif

#ifndef S_CRITICAL
#define S_CRITICAL  0x20        //< @def CRITICAL Severity level.
#endif

#ifndef S_ALERT
#define S_ALERT     0x40        //< @def ALERT Severity level.
#endif

#ifndef S_EMERGENCY
#define S_EMERGENCY 0x80        //< @def EMERGENCY Severity level.
#endif


// [ System Includes ]
#include <sstream>
#include <string>


//====================================================================================//

/**
* @class	LogReport LogService.h "inc/Log_Service.h"
* @brief	Log formatted report.
* 
* @details  Log formatted report of extended report logging.
*/
class EXPORT LogReport
{

    friend class Pantheios_Logger;

public:

    /**
    * @enum Report's Log level.
    * \n */
    enum LogType
    { L_DEBUG
    , L_INFO
    , L_NOTICE
    , L_WARNING
    , L_ERROR
    , L_CRITICAL
    , L_ALERT
    , L_EMERGENCY
    };

    inline

        /**
        * @brief        LogReport constructor.
        * @param[in]    Log level.
        * @param[in]    Log Report Topic message.
        * \n */
        LogReport
        ( const LogType&            __rLogType
        , const std::string&        __rTopic
        )
        : m_logType( __rLogType )
        , m_topic( __rTopic )
    {   }

    LogReport( const LogReport& __right );              //! Copy CTOR.

    LogReport& operator=(const LogReport& __right );    //! Assignment operator.

    /**
    * @brief        Returns LogReport's input streamer in order to push a message to it.
    * @return       Returns a reference to LogReport's input streamer.
    * \n */
    inline
        std::ostringstream& getrDetailStream() const { return m_details; }

    /**
    * @brief        Given a severity message, it pushes LogReport data to given streambuffer.
    * @param[in]    __rSeverityMsg      Severity message to be added to LogReport's stream.
    * @param[in]    __RetStreamBuf      Given a streambuffer reference, a logreport message is streamed inside it.
    * \n */
    void getFormattedReport( const std::string& __rSeverityMsg, std::ostringstream& __rRetStreamBuf ) const ;

public:

    /**
    * @brief        Serializes data from LogReport to a buffer (F.E. ready to be sent over network )
    * @param[out]   __rReturnedsize     Returns size of returned buffer.
    * \n*/

    unsigned char* serialize(unsigned& __rReturnedSize)  const;

    /**
    * @brief        Returns a reference to groups' name.
    * @return       Returns source's name.
    * \n */
    std::string&    getSourceRef() const;

    /**
    * @brief        Returns a reference to process ID.
    * @return       Process ID.
    * \n */
    __int32&    getProcIDRef() const;


    /**
    * @brief        Returns a reference to process ID.
    * @return       Process ID.
    * \n */
    __int64&         getUTCtime() const;


    /**
    * @brief        Returns LogReport's severity level.
    * @return       SeverityLevel.
    * \n */
    const LogType  getLogType() const;


    /**
    * @brief        Returns a reference to LogReport topic.
    * @return       LogReport topic.
    * \n */
    const std::string&    getTopicRef() const;

    static bool unserialize
        ( const unsigned char*  __pInBuffer
        , const unsigned&       __rInBufferSize
        , LogReport*&           __rpReturnedLog
        );

private:

    const LogType                   m_logType;
    const std::string               m_topic;

    mutable __int64                 m_date;
    mutable __int32                 m_procID;
    mutable std::string             m_source;

    mutable std::ostringstream      m_details;

};



//====================================================================================//

/**
* @class	ILogServiceException LogService.h "inc/LogService.h"
* @brief	LogService exception.
* 
* @details   LogService exception object.
*/
class EXPORT ILogServiceException
{

public:
    /**
    * @brief         LogService Constructor.
    * @param[in]     _message   Exception message
    * @param[in]     _file      .cpp source file containing exception code.
    * @param[in]     _line      .cpp source file line at which exception was thrown.
    * \n */
    ILogServiceException(const std::string& _message, const std::string& _file, const unsigned long& _line );
    /**
    * @details      Gets exception message.
    * @return     string containing exception message.
    * \n */
    std::string getMsg();


private:
    const std::string m_message;            //!<    Exception message.
    const std::string m_file;               //!<    .cpp file containing code that threw exception.
    const unsigned long m_line;             //!<    line at which exception was thrown

};


//====================================================================================//

/**
* @class	ILogService Log_Service.h "inc/Log_Service.h"
* @brief	LogService exception.
* 
* @details  LogService exception object.
*/

class EXPORT ILogService
{
    friend class LogFactory;

public:

    enum LoggerStatus { DEFAULT, INITED, STOPPED, STARTED };

    enum ReturnValues { FAILURE, WRONG_CONFIGURATION, NOT_INITED, ALREADY_INITED, NOT_STARTED, ALREADY_STARTED, SUCCESS };


    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    /**
    * @class	ILogCfg Log_Service.h "inc/Log_Service.h"
    * @brief	Generic configuration property set.
    * 
    * @details  Used by Factory and by Log_Service to instantiate logging system.
    */
    class EXPORT ILogCfg
    {
    public:

        /**
        * @enum LogToFlags  Backend identification { LOG_TO_CONSOLE, LOG_TO_FILE }
        * \n */
        enum LogToFlags
        {
            CFG_LOGTOCONSOLE    = 0x01
            , CFG_LOGTOFILE     = 0x02
        };

        // Default CTOR
        ILogCfg() : m_config(0x00 ), m_fileFilterMask( 0x00 ), m_consoleFilterMask( 0x00 ) { }

        // Virtual DTOR
        virtual ~ILogCfg() { }

        /**
        * @brief         Sets process ID.
        * @param[in]     __rProcessName         Process ID to be set.
        * \n */
        void setProcessName( const std::string& __rProcessName );

        /**
        * @brief         Sets log file path.
        * @param[in]     __rLogFilePath         File path to log file.
        * \n */
        void setLogFilePath( const std::string& __rLogFilePath );

        /**
        * @brief         Adds to a specific log backend a severity property mask.
        * @param[in]     __rBackendFlag         Service definition which has to be loaded.
        * @param[in]    __rSeverityMask Constructed service or plugin.
        * @return        Return code.
        *\n */
        void addConfigProperty
            ( const unsigned char& __rBackendFlag
            , const unsigned char& __rSeverityMask
            );

        /**
        * @brief         Given a specificy log backend ID, it yelds its proper mask.
        * @param[in]     __rBackendItem         Backend ID.
        * @return        Severity mask.
        \n */
        unsigned char   getConfigProperty( const unsigned char& __rBackendItem ) const;

        /**
        * @brief        Returns actually set process ID.
        * @return       Process ID.
        \n */
        std::string getProcessName( ) const;

        /**
        * @brief        Returns log file path.
        * @return       File path to log file.
        \n */
        std::string  getLogFilePath() const;

        /**
        * @brief        Returns backend configuration mask.
        * @return       Configuration mask.
        \n */
        unsigned char  getConfigMask( ) const;

    private:
        std::string             m_processName;              //! Process ID.
        std::string             m_filepath;                 //! Log file path.
        unsigned char           m_config;                   //! Configured log backend mask.

        unsigned char           m_consoleFilterMask;        //! Console severity level mask.
        unsigned char           m_fileFilterMask;           //! Log to file severity mask.

    };

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://


    ILogService()                                           //! Default CTOR.
        : m_loggerStatus(DEFAULT )
        , m_pCfg( 0 )
    { }

    virtual ~ILogService();                                 //! Default virtual DTOR.

    /**
    * @brief        DEBUG Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_DEBUG(const LogReport& __rReport) = 0;

    /**
    * @brief        DEBUG Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_DEBUG(const char* format, ...) = 0;

    /**
    * @brief        INFO Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_INFO(const LogReport& __rReport) = 0;

    /**
    * @brief        INFO Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_INFO(const char* format, ...) = 0;

    /**
    * @brief        NOTICE Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_NOTICE(const LogReport& __rReport) = 0;

    /**
    * @brief        NOTICE Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_NOTICE(const char* format, ...) = 0;

    /**
    * @brief        WARNING Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_WARNING(const LogReport& __rReport) = 0;

    /**
    * @brief        WARNING Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_WARNING(const char* format, ...) = 0;

    /**
    * @brief        ERROR Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_ERROR(const LogReport& __rReport) = 0;

    /**
    * @brief        ERROR Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_ERROR(const char* format, ...) = 0;

    /**
    * @brief        CRITICAL Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_CRITICAL(const LogReport& __rReport) = 0;

    /**
    * @brief        CRITICAL Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_CRITICAL(const char* format, ...) = 0;

    /**
    * @brief        ALERT Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_ALERT(const LogReport& __rReport) = 0;

    /**
    * @brief        ALERT Log message action on.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_ALERT(const char* format, ...) = 0;

    /**
    * @brief        EMERGENCY Log message action on a report.
    * @param[in]    __rReport   Report to be logged.
    \n */
    virtual void Log_EMERGENCY(const LogReport& __rReport) = 0;

    /**
    * @brief        EMERGENCY Log message action.
    * @param[in]    format  message to be logged.
    \n */
    virtual void Log_EMERGENCY(const char* format, ...) = 0;

public:

    virtual const ILogCfg*    getConfigCfg() const = 0;

public:

    static  ILogService* getPtr() throw( ILogServiceException );    


    ////////////// FOLLOWING METHODS ARE USED BY LOGFACTORY CLASS ////////////
public:
    virtual ReturnValues init( const ILogCfg* __pConfig ) = 0;

    virtual ReturnValues start() = 0;

    virtual ReturnValues stop() = 0;

    //////////////////////////////////////////////////////////////////////

protected:
    LoggerStatus            m_loggerStatus;
    const ILogCfg           *m_pCfg;


protected:
    static ILogService* PInstance;

};




//====================================================================================//


/**
* @class	LogFactoryException Log_Service.h "inc/Log_Service.h"
* @brief	LogFactoryException exception.
* 
* @details  LogFactoryException exception object.
*/

class EXPORT LogFactoryException
{
public:

    /**
    * @details      LogFactoryException CTOR
    * @param[in]    __rMessage      Exception message.
    * @param[in]    __rFile         Filename.
    * @param[in]    __rLine         Line where exception was thrown.
    * \n */
    LogFactoryException(const std::string& __rMessage, const std::string& __rFile , const unsigned long& __rLine );
    std::string getMsg();

private:
    const std::string m_message;            //! Exception message.
    const std::string m_file;               //! File where exception occoured.
    const unsigned    m_line;               //! Line where exception occoured.
};



//====================================================================================//


/**
* @class	LogFactory Log_Service.h "inc/Log_Service.h"
* @brief	Factory object responsable to wrap Log Service creation strategy.
*
* 
* @details  Creates a Log_Service object.
*/

class EXPORT LogFactory
{
public:

    enum Status { FACTORY_NOT_INITED, LOGGER_NOT_INITED, LOG_SYSTEM_INITED, LOG_SYSTEM_STARTED };

public:

    enum Loggertype{ PANTHEIOS };

    LogFactory();

    virtual ~LogFactory();

    void init(const Loggertype& __rLoggerType ) throw( LogFactoryException );

    ILogService::ILogCfg* GetConfigTool() const throw( LogFactoryException );

    void InitLogger(  ILogService::ILogCfg* const __pConfig ) throw( LogFactoryException );

    void StartLogger() throw( LogFactoryException );

    void StopLogger( ) throw( LogFactoryException );

    static void Destroy();

    inline Status getStatus() const { return m_factoryStatus; }

public:
    static LogFactory*  getPtr();

protected:
    static LogFactory* PFactoryInstance;

private:
    Status      m_factoryStatus;

};


