

#include <iostream>
#include <string>
#include <vector>

#include <INetBroker.h>

#include "dispatcherTester.h"


DispatcherTester::DispatcherTester( IDispatcher*& _disp
                                   , Modality _mod
                                   , boost::mutex* __rLogMutex
                                   , std::vector<Group> _groups
                                   , boost::mutex& __rTasksMutex
                                   , boost::condition_variable& __rCondClose
                                   , int& _task )
                                   : m_pDispatcher(_disp)
                                   , m_pThread(0 )
                                   , m_mod( _mod )
                                   , m_pLogMutex( __rLogMutex )
                                   , m_groups(_groups)
                                   , m_rTasksMutex(__rTasksMutex)
                                   , m_rCondClose(__rCondClose)
                                   , m_rTasks( _task )

{

}

DispatcherTester::~DispatcherTester()
{

    delete m_pThread;


}

void DispatcherTester::start()
{

    m_pThread =  new boost::thread(boost::bind(&DispatcherTester::job, this) );

}


void DispatcherTester::sendToDispatcher()
{
    std::string test = "prova", test2 = "prova2doodof";

    int ret = 0;

    for( unsigned int i = 0; i < m_groups.size() ; ++i )
    {

        {
            boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
            std::cout << "Sending message " << test << " to " << m_groups[i].getName();
        }


        std::string name = m_groups[i].getName();

        try
        {
            ret = m_pDispatcher->sendMessageToGroup( m_pDispatcher->makeMessage(name
                , test.size() + 1, test.c_str(), 0x0001 ) );

            {
                boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
                std::cout << " return code: " << ret << std::endl;
            }
        } catch ( DispatcherException& ex )
        {
            boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
            std::cout << ex.getMsg() << std::endl;           

        }

        {
            boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
            std::cout << "Sending message " << test2 << " to " << m_groups[i].getName();
        }


        try
        {

            ret =  m_pDispatcher->sendMessageToGroup(m_pDispatcher->makeMessage(name
                , test2.size() + 1, test2.c_str(), 0x0002 ) );

            {
                boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
                std::cout << " return code: " << ret << std::endl;
            }

        } catch ( DispatcherException& ex )
        {
            boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
            std::cout << ex.getMsg() << std::endl;           

        }



    }
}

void DispatcherTester::receiveFromDispatcher( )
{

    static Buffer buf;
    bool retVal = 0;

    for( unsigned int i = 0; i< m_groups.size(); ++i )
    {

        std::string name = m_groups[i].getName();

        retVal = m_pDispatcher->receiveMessageFromGroup(name, buf, true );
        boost::this_thread::sleep( boost::posix_time::milliseconds(50) );

        if( retVal == true )
        {
            boost::unique_lock<boost::mutex> lock(*m_pLogMutex);
            std::cout << "Received message: " << buf.getPtr() << " from group "
                << buf.getGroupName() << std::endl;
        }
    }
}

void DispatcherTester::job(void)
{
    //int iterations = 20;

    while( /*--iterations*/ true )
    {
        if( m_mod == PRODUCER )
        {

            sendToDispatcher();


            boost::this_thread::sleep( boost::posix_time::milliseconds(500) );

        } else if ( m_mod == CONSUMER )
        {

            receiveFromDispatcher();

            boost::this_thread::sleep( boost::posix_time::milliseconds(50) );
        }


        
    }

    boost::unique_lock<boost::mutex> lock(m_rTasksMutex);
    --m_rTasks;

    m_rCondClose.notify_one();
}