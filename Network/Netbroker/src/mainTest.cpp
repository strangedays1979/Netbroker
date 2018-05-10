

#pragma warning(disable: 4996)

#include <iostream>

#include <boost/thread.hpp>

#include <syncFifoQueue.h>

#include <INetBroker.h>
#include <SpreadConnector.h>

#include "queueTester.h"
#include "dispatcherTester.h"


#define MSGBUFFER 1000


static	void	Usage(int argc, char *argv[]);


static void listen();
static void keepListen();

static void send();
static void exitConnector();
static void join();
static void leave();

static void testQueue();
static void testConnector();
static void testDispatcher();

static IConnector* Conn             = 0;
static	char	User[80]            = {'\0' };
static  char    Spread_name[80]     = {'\0' };
static char     Loglevel[80]        = {'\0' };

static Verbosity g_verbosity;

#if defined(_LOGSERVICE)
static bool initLogSystem(ILogService::ILogCfg*& __pCfg );
static bool startLogService(ILogService::ILogCfg* const __pCfg);
static void haltLogSystem();
#endif




int _cdecl main( int argc, char* argv[] )
{

    Usage( argc, argv );


#if defined(_LOGSERVICE)

    ILogService::ILogCfg *pCfg = 0;

    if( !initLogSystem(pCfg ) )
        return false;

    if(!strcmp(Loglevel, "low" ))
    {
        g_verbosity = Verbosity::LOW;

        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOCONSOLE
            , S_ERROR | S_ALERT | S_CRITICAL | S_EMERGENCY );


    } else if( !strcmp(Loglevel, "medium" ))
    {
        g_verbosity = Verbosity::MEDIUM;

        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOCONSOLE
            , S_ERROR | S_ALERT | S_CRITICAL | S_EMERGENCY | S_WARNING | S_NOTICE | S_INFO );



    } else if( !strcmp(Loglevel, "high" ))
    {        
        g_verbosity = Verbosity::HIGH;

        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOCONSOLE
            , S_ERROR | S_ALERT | S_CRITICAL | S_EMERGENCY | S_WARNING | S_NOTICE | S_INFO | S_DEBUG );

        pCfg->addConfigProperty( ILogService::ILogCfg::CFG_LOGTOFILE, S_ERROR | S_ALERT | S_CRITICAL | S_EMERGENCY | S_WARNING | S_NOTICE | S_INFO | S_DEBUG );


    }

    pCfg->setProcessName( std::string("test_NB"));


    if( !startLogService(pCfg) )
        return false;


#endif

    char choice;

    do
    {
        std::cout << "\n\n============= NETBROKER TEST APPLICATION ==============" << std::endl;
        std::cout << "a) Queue Test" << std::endl;
        std::cout << "b) IConnector Test" << std::endl;
        std::cout << "c) Dispatcher Test" << std::endl;
        std::cout << "x) Exit\n\n";
        std::cout << "Which is your choice? > ";
        std::cin >> choice;

        if( (choice == 'a' ) || ( choice == 'A' ) )
        {
            testQueue();
        }
        else if( (choice == 'b' ) || ( choice== 'B' ) )
        {
            testConnector();
        }
        else if( ( choice == 'c' ) || ( choice == 'C' ) )
        {
            testDispatcher();           
        }
        else if( ( choice == 'x' ) || ( choice == 'X' ) )
        {
            return 0;

        } else continue;

    } while(1);

}


static	void	Usage(int argc, char *argv[])
{

    /* Setting defaults */
    sprintf( User, "simple" );
    sprintf( Spread_name, "3333");
    while( --argc > 0 )
    {
        argv++;

        if( !strncmp( *argv, "-u", 2 ) )
        {
            strcpy( User, argv[1] );
            argc--; argv++;
        }else if( !strncmp( *argv, "-s", 2 ) ){
            strcpy( Spread_name, argv[1] ); 
            argc--; argv++;
        }else if( !strncmp( *argv, "-l", 2 ) ){
            strcpy( Loglevel, argv[1] ); 
            argc--; argv++;
        }else{
            printf( "Usage: user\n%s\n%s\n%s\n",
                "\t[-u <user name>]  : unique (in this machine) user name",
                "\t[-s <address>]    : either port or port@machine",
                "\t[-l <verbosity>   : low, medium or high" );
                exit(1);
        }
    }
}

static void testConnector()
{
    std::cout << "Setting up client:" << std::endl;


    std::cout << "Creating IConnector..." << std::endl;

    Conn = GetConnector(std::string(User), g_verbosity , ConnectorType::SPREAD );

    std::cout << "Connecting IConnector..." << std::endl;

    int retVal = 0;


    if( (retVal = Conn->connectNetwork( std::string(Spread_name) ) < 0 ) )
    {
        std::cout << "Error connecting network. Error code: " << retVal << std::endl;
        delete Conn;
        Conn = 0;

        return ;
    }

    char c = 0;

    do {

        std::cout << "\n\nEnter your choice:" << std::endl;
        std::cout << "j) Join a group. " << std::endl;
        std::cout << "l) Leave a group. " << std::endl;
        std::cout << "s) Send a message to group." << std::endl;
        std::cout << "r) Receive." << std::endl;
        std::cout << "k) Keep Receive." << std::endl;
        std::cout << "x) Disconnect and exit.\n\n" << std::endl;
        std::cout << "Which is your choice? > ";
        std::cin >> c;

        if( (c == 's' ) || ( c == 'S' ) )
        {

            send();


        }
        else if( (c == 'r' ) || ( c== 'R' ) )
        {

            listen();



        } else if( ( c == 'x' ) || ( c == 'X' ) )
        {

            exit(1);            

        } else if( ( c =='j' ) || ( c== 'J' ) )
        {

            join();   

        } else if ( (c== 'l') || ( c== 'L' ) )
        {

            leave();   


        } else if ( (c== 'k') || ( c== 'K' ) )
        {

            keepListen();

        } else if( (c== 'x' ) || ( c == 'X' ) )
        {

            exitConnector();

        }else continue;

    } while( 1);




}



static void listen()
{


    Buffer buf;

    std::cout << "\nWaiting for message... ";

    int retVal = 0;

    if( ( retVal = Conn->receiveFromGroup(buf) <0 ) )
    {
        std::cout << "Wops something bad happened: Error code: " << retVal;

    }

    std::cout << "Message received: " << buf.getPtr() << std::endl;
}



static void keepListen()
{
    while(1)
    {


        listen();

    }
}

static void send()
{
    char message[MSGBUFFER] = { '\0' };
    unsigned long buffSize = MSGBUFFER;
    static char groupName[CONN_MAX_GROUP_NAME] = {'\0'};

    std::cout << "Enter group you want to send a message to: ";
    std::cin >> groupName;

    std::cout << "\nEnter your message: ";
    std::cin >> message;

    std::string messageToSend(message);

    unsigned long size = messageToSend.size();

    Buffer buf(size + 1, messageToSend.c_str() , groupName, 0x0001 );
    int retVal = 0;

    if( (retVal = Conn->sendToGroup(buf ) ) < 0 )
    {
        std::cout << "Wops something bad happened: Error code: " << retVal;

    }
    retVal = 0;

    memset(message, 0x00, MSGBUFFER);
    memset(groupName, 0x00, CONN_MAX_GROUP_NAME);

}


static void exitConnector()
{

    static int retVal = 0;
    if( ( retVal = Conn->disconnectNetwork() ) < 0 )
    {
        std::cout << "Whops.. something bad happened: ErrorCode: " << retVal << std::endl;
        delete Conn;
        Conn =0;
    }
}



static void join()
{
    static char groupName[CONN_MAX_GROUP_NAME] = {'\0'};

    std::cout << "Enter group name you would like to join > ";
    std::cin >> groupName;

    std::cout << std::endl << "You entered: " << groupName << std::endl;

    std::cout << "Joining group " <<  groupName << std::endl;
    int retVal = 0;

    if( ( retVal = Conn->joinGroup( std::string(groupName) ) ) < 0 )
    {
        std::cout << "Error joining a group. ErrorCode: " << retVal << std::endl;
    }

    memset( groupName, 0x00, CONN_MAX_GROUP_NAME );


}

static void leave()
{
    static char groupName[CONN_MAX_GROUP_NAME] = {'\0'};
    std::cout << "Enter group name you would like to leave > ";
    std::cin >> groupName;

    std::cout << std::endl << "You entered: " << groupName << std::endl;
    std::cout << "Joining group " <<  groupName << std::endl;
    int retVal = 0;

    if( ( retVal = Conn->leaveGroup( std::string(groupName) ) ) < 0 )
    {
        std::cout << "Error joining a group. ErrorCode: " << retVal << std::endl;
    }

    memset( groupName, 0x00, CONN_MAX_GROUP_NAME );

}


static void testQueue()
{

    SyncFifoQueue<int> test(10);

    boost::mutex logMutex;
    boost::mutex tasksMutex;
    int tasksAttivi = 0;

    static boost::condition_variable cond;
    static boost::mutex mainMutex;

    Processor produttore("Produttore", tasksAttivi, Processor::PRODUCER, test, logMutex, tasksMutex, cond, 5, 300);
    Processor consumatore("Consumatore", tasksAttivi, Processor::CONSUMER, test, logMutex, tasksMutex, cond, 500 );

    boost::unique_lock<boost::mutex> lock(mainMutex);
    while(!tasksAttivi)
    {
        cond.wait(lock);
    }


}


static void testDispatcher()
{



    static char groupName[CONN_MAX_GROUP_NAME] = { '\0' };
    unsigned long size, qsize;
    char addOneMore;
    std::vector<Group> groups;
    static boost::mutex logMutex;

    char modality;

    boost::mutex tasksMutex;
    int tasksAttivi = 1;

    static boost::condition_variable cond;
    static boost::mutex mainMutex;

    do {
        std::cout << "\n======= Dispatcher test program =======" << std::endl;
        std::cout << "a) Only receive.\nb) Only send.\n\n";
        std::cout << "Which is your choice? >";
        std::cin >> modality;

        if( ( modality == 'a' ) || ( modality  == 'A' ) || ( modality == 'b' ) || ( modality == 'B' ) )
            break;    
        else continue;

    } while (1 );

    IDispatcher *pDisp = GetDispatcher( std::string(User), std::string(Spread_name),  ConnectorType::SPREAD, g_verbosity, false, false );

    if( !pDisp->connect() )
    {
        std::cout << "Impossible to connect network\n\n";
        return;
    }

    do
    {

        std::cout << "\n======== Add New Groups to Dispatcher Tester ========" << std::endl;
        std::cout << "Group name > ";
        std::cin >> groupName;
        std::cout << "Group rcv buffer size > ";
        std::cin >> size;
        std::cout << "queue buffer size > ";
        std::cin >> qsize;
        std::cout << "Creating group... " << std::endl;

        groups.push_back( Group(std::string( groupName ), size, qsize ) );


        try
        {
            if( ( modality == 'a' ) || ( modality  == 'A' ) )
                pDisp->registerGroup(std::string( groupName ), size, qsize );
            else if( ( modality == 'b' ) || ( modality  == 'B' ) )
                pDisp->registerGroup(std::string( groupName ), size, qsize, true );

        } catch (DispatcherException& ex )
        {
            std::cout << "Not Connected: " << ex.getMsg().c_str() << std::endl;

        }


        std::cout << "Add one more group press 'y' to add a new one? > ";
        std::cin >> addOneMore;

        memset( groupName , 0x00 , CONN_MAX_GROUP_NAME );

        if( ( addOneMore == 'y' ) || ( addOneMore == 'Y' ) )
            continue;

        std::cout << "\n\n";
        break;


    } while( 1 );

    if( ( modality == 'b' ) || ( modality  == 'B' ) )
    {
        DispatcherTester *pDProducer = new DispatcherTester( pDisp, DispatcherTester::PRODUCER, &logMutex, groups, tasksMutex, cond, tasksAttivi);
        pDisp->start() ;
        pDProducer->start();

    } else if( ( modality == 'a' ) || ( modality == 'A' )  )
    {
        DispatcherTester *pDConsumer = new DispatcherTester( pDisp, DispatcherTester::CONSUMER, &logMutex, groups, tasksMutex, cond, tasksAttivi );
        pDisp->start();
        pDConsumer->start();
    }

    boost::unique_lock<boost::mutex> lock(mainMutex);
    while(tasksAttivi>0)
    {
        cond.wait(lock);
    }

    pDisp->stop();

    pDisp->disconnect();

    if( pDisp )
        delete pDisp;

}

#if defined(_LOGSERVICE)


bool initLogSystem(ILogService::ILogCfg*& __pCfg )
{

    if( LogFactory::getPtr()->getPtr()->getStatus() == LogFactory::LOG_SYSTEM_STARTED )
        return false;

    try
    {
        LogFactory::getPtr()->init(LogFactory::PANTHEIOS);
    } catch(LogFactoryException &ex )
    {
        printf("\nUnable to init Pantheios logger: %s\n", ex.getMsg().c_str() );

        return false;
    }

    try
    {
        __pCfg = LogFactory::getPtr()->GetConfigTool();


    } catch(LogFactoryException &ex )
    {

        printf("\nUnable to get Initialization Pantheios struct: %s\n", ex.getMsg().c_str() );

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

        printf("\nUnable to init logger: %s\n", ex.getMsg().c_str() );

        return false;
    }


    try
    {
        LogFactory::getPtr()->StartLogger();

    } catch(LogFactoryException &ex )
    {

        printf("\nUnable to start logger: %s\n", ex.getMsg().c_str() );

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
        printf("\nUnable to stop logger: %s\n", ex.getMsg().c_str() );

    }

    LogFactory::getPtr()->Destroy();



}

#endif