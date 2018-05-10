

/**
* @file         Log_Service.cpp
* @brief        Contains Log_Service implementations.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/




#include "Log_Service.h"


ILogService* ILogService::PInstance             = 0;
LogFactory*  LogFactory::PFactoryInstance       = 0;


ILogService* ILogService::getPtr()  throw( ILogServiceException )
{
    if( !PInstance )
        throw ILogServiceException("ILogService not initialized!", __FILE__, __LINE__);

    return PInstance;

}


LogFactory*  LogFactory::getPtr()
{

    if( !PFactoryInstance )
    {
        PFactoryInstance = new LogFactory();
        return PFactoryInstance;
    }

    return PFactoryInstance;

}



ILogService::~ILogService()
{

    if( m_pCfg )
        delete m_pCfg;
 
}

#if defined( _DEBUG )
#define ___EXCEPTION_PRINTMESSAGE                                                               \
    oss << "[" << m_file << ":" << m_line << "] " << m_message;
#else
#define IMPLEMENT_EXCEPTION_PRINTMESSAGE                                                            \
    oss << m_message << std::endl;
#endif


ILogServiceException::ILogServiceException(const std::string& _message
                                           , const std::string& _file 
                                           , const unsigned long& _line )
                                           : m_message(_message)         
                                           , m_file( _file )                                                                               
                                           , m_line( _line )                                                                               
{  }      


std::string ILogServiceException::getMsg()                                                                 
{                                                                                               
    std::ostringstream oss;                                                                         
    ___EXCEPTION_PRINTMESSAGE                                                                       
        return oss.str();                                                                               
}


/**
* Adopted by Log Request Manager
* \n */
std::string&    LogReport::getSourceRef() const
{
    return m_source;
}

__int32&    LogReport::getProcIDRef() const
{
    return m_procID;
}

__int64&         LogReport::getUTCtime() const
{
    return m_date;
}

/**************************/

const LogReport::LogType  LogReport::getLogType() const
{
    return m_logType;
}

const std::string&    LogReport::getTopicRef() const
{
    return m_topic;
}


void ILogService::ILogCfg::setProcessName( const std::string& __rProcessName ) { m_processName = __rProcessName; }

void ILogService::ILogCfg::setLogFilePath( const std::string& __rLogFilePath ) { m_filepath = __rLogFilePath; }

std::string ILogService::ILogCfg::getProcessName( ) const
{
    return m_processName;
}

std::string  ILogService::ILogCfg::getLogFilePath() const
{
    return m_filepath;
}

unsigned char  ILogService::ILogCfg::getConfigMask( ) const
{
    return m_config;
}


void ILogService::ILogCfg::addConfigProperty
            ( const unsigned char& __rBackendItem
            , const unsigned char& __rSeverityMask
            )
{

    switch( __rBackendItem )
    {

    case CFG_LOGTOCONSOLE:

        m_config            |= __rBackendItem;
        m_consoleFilterMask |= __rSeverityMask;

        break;

    case CFG_LOGTOFILE:

        m_config            |= __rBackendItem;
        m_fileFilterMask    |= __rSeverityMask;

        break;

    default:

        break;
    }


}


unsigned char   ILogService::ILogCfg::getConfigProperty( const unsigned char& __rBackendItem ) const
{

    
    switch( __rBackendItem )
    {

    case CFG_LOGTOCONSOLE:
        return m_consoleFilterMask;

    case CFG_LOGTOFILE:
        return m_fileFilterMask ;

        break;

    default:

        break;
    }


    return 0;




}
