

/**
* @file         LogFactory.cpp
* @brief        Contains Log_Service Factory implementations.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/



#include "Log_Service.h"


#include "Pantheios_Logger.h"

#include <sstream>


#if defined( _DEBUG )
    #define ___EXCEPTION_PRINTMESSAGE                                                               \
        oss << "[" << m_file << ":" << m_line << "] " << m_message;
#else
#define IMPLEMENT_EXCEPTION_PRINTMESSAGE                                                            \
        oss << m_message << std::endl;
#endif

LogFactoryException::LogFactoryException(const std::string& _message
    , const std::string& _file
    , const unsigned long& _line )
    : m_message(_message)
    , m_file( _file )
    , m_line( _line )
    {  }
    std::string LogFactoryException::getMsg()
    {
    std::ostringstream oss;
    ___EXCEPTION_PRINTMESSAGE                                                                     
    return oss.str(); 
    }


LogFactory::LogFactory()
: m_factoryStatus( FACTORY_NOT_INITED )
{



}

LogFactory::~LogFactory()
{


}

void LogFactory::init(const Loggertype& __rLoggerType ) throw( LogFactoryException )
{
    if( m_factoryStatus != FACTORY_NOT_INITED )
        throw LogFactoryException("Factory already inited!", __FILE__, __LINE__ );

    switch( __rLoggerType )
    {
    case PANTHEIOS:
        {

            ILogService::PInstance = new Pantheios_Logger;

            break;
        }
    default:
        {



            break;
        }
    }



    m_factoryStatus = LOGGER_NOT_INITED;

}


ILogService::ILogCfg* LogFactory::GetConfigTool() const
throw( LogFactoryException )
{

    if( m_factoryStatus == FACTORY_NOT_INITED )
        throw LogFactoryException("Factory not inited!", __FILE__, __LINE__ );


    return (ILogService::ILogCfg*) ILogService::getPtr()->getConfigCfg();


}


void LogFactory::InitLogger(  ILogService::ILogCfg* const __pConfig )
throw( LogFactoryException )
{
    if( m_factoryStatus == FACTORY_NOT_INITED )
        throw LogFactoryException("Factory not inited!", __FILE__, __LINE__ );

    if( m_factoryStatus == LOG_SYSTEM_INITED )
        throw LogFactoryException("Logger already inited!", __FILE__, __LINE__ );        

    ILogService::ReturnValues retVal;

    if( (retVal =ILogService::getPtr()->init(__pConfig)) == ILogService::FAILURE )
        throw LogFactoryException("Failed to initiate logger", __FILE__, __LINE__ );

    if( retVal == ILogService::ALREADY_INITED )
        throw LogFactoryException("Logger already inited!", __FILE__, __LINE__ );


    m_factoryStatus = LOG_SYSTEM_INITED;


}


void LogFactory::StartLogger( )
throw( LogFactoryException )
{
    if( m_factoryStatus == FACTORY_NOT_INITED )
        throw LogFactoryException("Factory not inited!", __FILE__, __LINE__ );

    if( m_factoryStatus != LOG_SYSTEM_INITED )
        throw LogFactoryException("Logger not inited!", __FILE__, __LINE__ );        

    ILogService::ReturnValues retVal;

    if( (retVal =ILogService::getPtr()->start()) != ILogService::SUCCESS )
        throw LogFactoryException("Failed to start logger", __FILE__, __LINE__ );

    m_factoryStatus = LOG_SYSTEM_STARTED;


}

void LogFactory::StopLogger( )
throw( LogFactoryException )
{
    if( m_factoryStatus != LOG_SYSTEM_STARTED )
        throw LogFactoryException("Logger not started!", __FILE__, __LINE__ );

    ILogService::ReturnValues retVal;

    if( (retVal =ILogService::getPtr()->stop()) != ILogService::SUCCESS )
        throw LogFactoryException("Failed to stop logger", __FILE__, __LINE__ );


    m_factoryStatus = LOG_SYSTEM_INITED;

}


void LogFactory::Destroy()
{

    try
    {
        if( getPtr() )
            getPtr()->StopLogger();

    } catch( LogFactoryException &ex )
    {



    }

    if( ILogService::PInstance )
    {
        delete ILogService::PInstance;
        ILogService::PInstance = 0;
    }

    getPtr()->m_factoryStatus = FACTORY_NOT_INITED;

    delete getPtr();

}

