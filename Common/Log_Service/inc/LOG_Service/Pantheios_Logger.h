
//=====================================================================================================//
/**
* @file         Pantheios_Logger.h
* @brief        Pantheios log specialization.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/
//=====================================================================================================//



#pragma once

#define MAX_NUMBEROFBACKENDS   3

#include <boost/thread.hpp>

#include "Log_Service.h"


class Pantheios_Logger: public ILogService
{

public:

    class PantheiosCfg: public ILogCfg
    {

    public:
        PantheiosCfg() : ILogCfg() { }



    };

    Pantheios_Logger();

    virtual ~Pantheios_Logger();

    const ILogCfg*    getConfigCfg() const ;

    ///////////////////// [ Inherited from ILogService ] //////////////


    void Log_DEBUG(const LogReport& __rReport);
    void Log_DEBUG(const char* format, ...);

    void Log_INFO(const LogReport& __rReport);
    void Log_INFO(const char* format, ...);

    void Log_NOTICE(const LogReport& __rReport);
    void Log_NOTICE(const char* format, ...);

    void Log_WARNING(const LogReport& __rReport);
    void Log_WARNING(const char* format, ...);

    void Log_ERROR(const LogReport& __rReport);
    void Log_ERROR(const char* format, ...);

    void Log_CRITICAL(const LogReport& __rReport);
    void Log_CRITICAL(const char* format, ...);

    void Log_ALERT(const LogReport& __rReport);
    void Log_ALERT(const char* format, ...);

    void Log_EMERGENCY(const LogReport& __rReport);
    void Log_EMERGENCY(const char* format, ...);

    std::string getProcessName() const { return m_applicationName; }

private:
    ReturnValues init( const ILogCfg* __pConfig );

    ReturnValues start();

    ReturnValues stop();

    //////////////////////////////////////////////////////////////////////


private:

    void halt();

private:

    boost::mutex            m_logMutex;
    bool                    m_enableLogToFile;
    std::string             m_applicationName;


};

