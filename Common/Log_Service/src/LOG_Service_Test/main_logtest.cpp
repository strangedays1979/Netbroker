

/**
* @file         main_logtest.cpp
* @brief        Main Log_Service test.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/



#define LOGTEST_BUFFERSIZE  1024

#if defined(ENABLELEAKDETECTION)
#include <vld.h>
#endif


#include <Log_Service.h>


#include <boost/thread.hpp>


static bool initLogSystem(ILogService::ILogCfg*& __pCfg );
static bool startLogService(ILogService::ILogCfg* const __pCfg);
static void haltLogSystem();

static boost::mutex     g_jobMutex;
static bool             g_jobRunning = true;

static boost::thread *g_pThread = 0;


static  char    g_buffer[LOGTEST_BUFFERSIZE];



bool JobIsRunning()
{
    boost::unique_lock<boost::mutex> lock(g_jobMutex);
    return g_jobRunning;
}

void SetRunMode(const bool& __rEnabled )
{
    boost::unique_lock<boost::mutex> lock(g_jobMutex);
    g_jobRunning = __rEnabled;

}


void job()
{

    while( JobIsRunning() )
    {
        LOG_NOTICE(g_buffer);
    }



}


int main(int argc, char* argv[] )
{

    memset(g_buffer, 0x00, LOGTEST_BUFFERSIZE);

    memset(g_buffer, 'X', LOGTEST_BUFFERSIZE-1 );

    ILogService::ILogCfg* pCfg = 0;

    if( !initLogSystem(pCfg ) )
        return false;
    else
    {
        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOCONSOLE, 0xff );
        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOFILE, 0xff );
    }

    std::string     processName;

    pCfg->setProcessName("LOG_TEST");

    if( !startLogService(pCfg) )
        return false;  

    g_pThread = new boost::thread( boost::bind(&job) );

    char result = 0;

    printf("\n\n... press x to exit...\n");

    while( (result != 'X') && (result != 'x' ) )
    {
        result = getchar();
    }

    SetRunMode(false);

    g_pThread->join();

    delete g_pThread;


    system("PAUSE");

    return 1;
}


bool initLogSystem(ILogService::ILogCfg*& __pCfg )
{

    if( LogFactory::getPtr()->getPtr()->getStatus() == LogFactory::LOG_SYSTEM_STARTED )
        return false;

    try
    {
        LogFactory::getPtr()->init(LogFactory::PANTHEIOS);
    } catch(LogFactoryException &ex )
    {
        LOG_NOTICE("\nUnable to init Pantheios logger: %s\n", ex.getMsg().c_str() );

        return false;
    }

    try
    {
        __pCfg = LogFactory::getPtr()->GetConfigTool();


    } catch(LogFactoryException &ex )
    {

        LOG_NOTICE("\nUnable to get Initialization Pantheios struct: %s\n", ex.getMsg().c_str() );

        return false;
    }



    return true;


}


bool startLogService(ILogService::ILogCfg* const __pCfg)
{
    try
    {
        LogFactory::getPtr()->InitLogger(__pCfg );

    } catch(LogFactoryException &ex )
    {

        LOG_NOTICE("\nUnable to init logger: %s\n", ex.getMsg().c_str() );

        return false;
    }


    try
    {
        LogFactory::getPtr()->StartLogger();

    } catch(LogFactoryException &ex )
    {

        LOG_NOTICE("\nUnable to start logger: %s\n", ex.getMsg().c_str() );

        return false;
    }

    return true;



}


void haltLogSystem()
{
    try
    {
        LogFactory::getPtr()->StopLogger();

    } catch(LogFactoryException &ex )
    {
        LOG_NOTICE("\nUnable to stop logger: %s\n", ex.getMsg().c_str() );

    }

    LogFactory::getPtr()->Destroy();



}

