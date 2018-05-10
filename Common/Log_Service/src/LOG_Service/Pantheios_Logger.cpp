

/**
* @file         Pantheios_Logger.cpp
* @brief        Pantheios log specialization.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/




/* Standard C Header Files */
#include <stdlib.h> 

#include <process.h>

#include <sstream>

#include <platformstl/platformstl.h>                /* for platform discrimination */

/* Pantheios Header Files */
#include <pantheios/pantheios.hpp>                    /* main Pantheios C header file */
//#include <pantheios/frontends/fe.all.h>
#include <pantheios/backends/be.N.h>
#include <pantheios/backends/bec.file.h>
#include <pantheios/backends/bec.fprintf.h>




#include "Pantheios_Logger.h"



pan_be_N_t PAN_BE_N_BACKEND_LIST[MAX_NUMBEROFBACKENDS];

//{
//   PANTHEIOS_BE_N_STDFORM_ENTRY(1, pantheios_be_file, 0)
//    , PANTHEIOS_BE_N_STDFORM_ENTRY(2, pantheios_be_fprintf, 0)
//    //, PANTHEIOS_BE_N_STDFORM_ENTRY(3, pantheios_be_null, 0)
//    //#if defined(PLATFORMSTL_OS_IS_UNIX)
//    //  , PANTHEIOS_BE_N_STDFORM_ENTRY(4, pantheios_be_syslog, 0)
//    //#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
//    //  , PANTHEIOS_BE_N_STDFORM_ENTRY(4, pantheios_be_WindowsSyslog, 0)
//    //#endif /* OS */
//    //  , PANTHEIOS_BE_N_STDFORM_ENTRY(5, pantheios_be_file, 0)
//      , PANTHEIOS_BE_N_TERMINATOR_ENTRY
//};

const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[]      =   PANTHEIOS_LITERAL_STRING( "test" );

Pantheios_Logger::Pantheios_Logger()
: m_enableLogToFile(false )
{




}

Pantheios_Logger::~Pantheios_Logger()
{

    halt();    


}

const ILogService::ILogCfg*    Pantheios_Logger::getConfigCfg() const
{
    if( !ILogService::m_pCfg )
        return new PantheiosCfg();

    return m_pCfg;

}

void Pantheios_Logger::halt()
{

    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus == STARTED )
    {
        stop();
    }


    using namespace pantheios;

    /* Must uninitialise Pantheios.
    *
    * pantheios_uninit() must be called once for each successful (>=0)
    * invocation of pantheios_init().
    */
    pantheios_uninit();

    m_loggerStatus = DEFAULT;

}


PANTHEIOS_CALL(int) pantheios_fe_init(void*   reserved
                                      , void**  ptoken)
{
    *ptoken = NULL;

    return 0;
}

PANTHEIOS_CALL(void) pantheios_fe_uninit(void* token)
{}



PANTHEIOS_CALL(int) pantheios_fe_isSeverityLogged(void* token
                                                  , int   severity
                                                  , int   backEndId)
{

    
    unsigned char       severityMask = 0x00;

    switch( backEndId )
    {
    case 0:
        severityMask = ILogService::getPtr()->getConfigCfg()->getConfigProperty(ILogService::ILogCfg::CFG_LOGTOCONSOLE );
        break;

    case 1:         // FILE LOGGING

        severityMask = ILogService::getPtr()->getConfigCfg()->getConfigProperty(ILogService::ILogCfg::CFG_LOGTOFILE );
        break;

    case 2:         // CONSOLE LOGGING

        severityMask = ILogService::getPtr()->getConfigCfg()->getConfigProperty(ILogService::ILogCfg::CFG_LOGTOCONSOLE );
        break;

    default:
        return 0;
        break;

    }

    using namespace pantheios;


    switch(severity & 0x0f)
    {
    case  PANTHEIOS_SEV_EMERGENCY:
        return severityMask & S_EMERGENCY;

    case  PANTHEIOS_SEV_ALERT:
        return severityMask & S_ALERT;

    case  PANTHEIOS_SEV_CRITICAL:
        return severityMask & S_CRITICAL;

    case  PANTHEIOS_SEV_ERROR:
        return severityMask & S_ERROR;

    case  PANTHEIOS_SEV_WARNING:
        return severityMask & S_WARNING;

    case  PANTHEIOS_SEV_NOTICE:
        return severityMask & S_NOTICE;

    case  PANTHEIOS_SEV_INFORMATIONAL:
        return severityMask & S_INFO;

    case  PANTHEIOS_SEV_DEBUG:
        return severityMask & S_DEBUG;

    default:
        return 0x00;
        break;
    }

    return 0;

}

//
//PANTHEIOS_CALL(char const*) pantheios_fe_processIdentity(void* token)
//{
//    return ((Pantheios_Logger*) ILogService::getPtr())->getProcessName().c_str();
//}

PANTHEIOS_CALL(char const*) pantheios_fe_getProcessIdentity(void* token)
{
    static char data[64] = { '\0' };

    std::string tmp = ((Pantheios_Logger*) ILogService::getPtr())->getProcessName();
    strcpy(data, tmp.c_str() );

    return data;
}


ILogService::ReturnValues Pantheios_Logger::init( const ILogCfg* __pConfig )
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus == INITED )
        return ALREADY_INITED;

    if( !__pConfig )
        return WRONG_CONFIGURATION;

    m_pCfg = (ILogCfg*) __pConfig;

    using namespace pantheios;

    m_applicationName = m_pCfg->getProcessName();

    std::ostringstream finalFileName;

    if( m_applicationName.empty() )
        finalFileName << "noname";
    else
        finalFileName << m_applicationName;

    pan_be_N_t fileBackend = PANTHEIOS_BE_N_STDFORM_ENTRY(1, pantheios_be_file, 0);
    pan_be_N_t printfBackend = PANTHEIOS_BE_N_STDFORM_ENTRY(2, pantheios_be_fprintf, 0);
    pan_be_N_t backendTerminator = PANTHEIOS_BE_N_TERMINATOR_ENTRY;

    PAN_BE_N_BACKEND_LIST[0] = printfBackend;


    if( m_pCfg->getConfigMask() & ILogCfg::CFG_LOGTOFILE )
    {
        PAN_BE_N_BACKEND_LIST[1] = fileBackend;
        PAN_BE_N_BACKEND_LIST[2] = backendTerminator;
    }
    else
    {
        PAN_BE_N_BACKEND_LIST[1] = backendTerminator;
        PAN_BE_N_BACKEND_LIST[2] = backendTerminator;

    }


    if(pantheios_init() < 0)
        return FAILURE;


    if( m_pCfg->getConfigMask() & ILogCfg::CFG_LOGTOFILE )
    {

        char retBuf[128] = {'\0' };
        strftime ( retBuf, 128, "%m%d%y-%H%M", ::localtime(& (const time_t&)::time(NULL)) );
        finalFileName << '_' << _getpid() << '_' << retBuf << ".log";

        pantheios_be_file_setFilePath(PANTHEIOS_LITERAL_STRING(finalFileName.str().c_str()), PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BE_FILE_F_TRUNCATE, 1);

    }



    //pantheios_logputs(PANTHEIOS_SEV_DEBUG, PANTHEIOS_LITERAL_STRING("debug"));
    //pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL, PANTHEIOS_LITERAL_STRING("info"));
    //pantheios_logputs(PANTHEIOS_SEV_NOTICE, PANTHEIOS_LITERAL_STRING("notice"));
    //pantheios_logputs(PANTHEIOS_SEV_WARNING, PANTHEIOS_LITERAL_STRING("warn"));



    //pantheios_logputs(PANTHEIOS_SEV_ERROR, PANTHEIOS_LITERAL_STRING("error"));
    //pantheios_logputs(PANTHEIOS_SEV_CRITICAL, PANTHEIOS_LITERAL_STRING("critical"));
    //pantheios_logputs(PANTHEIOS_SEV_ALERT, PANTHEIOS_LITERAL_STRING("alert"));
    //pantheios_logputs(PANTHEIOS_SEV_EMERGENCY, PANTHEIOS_LITERAL_STRING("emergency"));

    //(pantheios_be_file_setFilePath(PANTHEIOS_LITERAL_STRING("file-5.log"), 0, 0, 5);

    //pantheios::log_DEBUG("debug");

    m_loggerStatus = INITED;

    return SUCCESS;




}


ILogService::ReturnValues Pantheios_Logger::start()
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != INITED )
        return NOT_INITED;

    if( m_loggerStatus == STARTED )
        return ALREADY_STARTED;


    /// code ///


    m_loggerStatus = STARTED;


    return SUCCESS;


}


ILogService::ReturnValues Pantheios_Logger::stop()
{

    boost::unique_lock<boost::mutex> lock(m_logMutex);


    if( m_loggerStatus != STARTED )
        return NOT_STARTED;

    /// code ///

    m_loggerStatus = INITED;


    return SUCCESS;


}



void Pantheios_Logger::Log_DEBUG(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("DEBUG", oss);

    pantheios::log_DEBUG(oss.str().c_str() );


}


void Pantheios_Logger::Log_DEBUG(const char* format, ...)
{

    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::debug,format,args);
    va_end (args);

}


void Pantheios_Logger::Log_INFO(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("INFO", oss);

    pantheios::log_INFORMATIONAL(oss.str().c_str() );

}

void Pantheios_Logger::Log_INFO(const char* format, ...)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;


    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::informational,format,args);
    va_end (args);

}

void Pantheios_Logger::Log_NOTICE(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("NOTICE", oss);

    pantheios::log_NOTICE(oss.str().c_str() );

}

void Pantheios_Logger::Log_NOTICE(const char* format, ...)
{

    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::notice,format,args);
    va_end (args);

}



void Pantheios_Logger::Log_WARNING(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("WARNING", oss);

    pantheios::log_WARNING(oss.str().c_str() );



}

void Pantheios_Logger::Log_WARNING(const char* format, ...)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;


    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::warning,format,args);
    va_end (args);

}


void Pantheios_Logger::Log_ERROR(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("ERROR", oss);

    pantheios::log_ERROR(oss.str().c_str() );

}

void Pantheios_Logger::Log_ERROR(const char* format, ...)
{

    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::error,format,args);
    va_end (args);

}


void Pantheios_Logger::Log_CRITICAL(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    struct tm * pTimeInfo = 0;
    pTimeInfo = localtime  ( &__rReport.getUTCtime() );

    std::ostringstream oss;

    __rReport.getFormattedReport("CRITICAL", oss);

    pantheios::log_CRITICAL(oss.str().c_str() );



}


void Pantheios_Logger::Log_CRITICAL(const char* format, ...)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;


    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::critical,format,args);
    va_end (args);

}


void Pantheios_Logger::Log_ALERT(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("ALERT", oss);

    pantheios::log_ALERT(oss.str().c_str() );



}

void Pantheios_Logger::Log_ALERT(const char* format, ...)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::alert,format,args);
    va_end (args);

}


void Pantheios_Logger::Log_EMERGENCY(const LogReport& __rReport)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    std::ostringstream oss;

    __rReport.getFormattedReport("EMERGENCY", oss);

    pantheios::log_EMERGENCY(oss.str().c_str() );



}



void Pantheios_Logger::Log_EMERGENCY(const char* format, ...)
{
    boost::unique_lock<boost::mutex> lock(m_logMutex);

    if( m_loggerStatus != STARTED )
        return;

    va_list args;
    va_start (args, format);
    pantheios::logvprintf(	pantheios::emergency,format,args);
    va_end (args);

}