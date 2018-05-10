

#include <iostream>

#include <SyncFifoQueue.h>

#include "queueTester.h"


Processor::Processor
( const std::string&            __rName
 , int&                         __rTasks
 , Type                         __type
 , SyncFifoQueue<int>&          __rQueue
 , boost::mutex&                __rLogMutex
 , boost::mutex&                __rTasksMutex
 , boost::condition_variable&   __rCondClose
 , const int&                   __rSleepTime
 , int                          __dataInts
 )
 : m_name( __rName)
 , m_rTasks( __rTasks )
 , m_type( __type )
 , m_pThread(0)
 , m_rQueue( __rQueue )
 , m_rTasksMutex(__rTasksMutex)
 , m_rCondClose(__rCondClose)
 , m_dataInts(__dataInts)
 , m_rLogMutex( __rLogMutex)
 , m_sleepTime( __rSleepTime )
{
    m_pThread = new boost::thread(boost::bind(&Processor::job, this) );

}




Processor::~Processor()
{

    delete m_pThread;
    m_pThread = 0;
}




void Processor::job()
{
    {
        boost::unique_lock<boost::mutex> lock(m_rTasksMutex);
        ++m_rTasks;
    }

    if( m_type == PRODUCER )
    {
        while( m_dataInts >= 0 )
        {
            m_rQueue.push( m_dataInts-- );
            {
                boost::unique_lock<boost::mutex> lock(m_rLogMutex);
                std::cout << "[PRODUCER] : " << m_dataInts + 1 << std::endl;
                boost::this_thread::sleep( boost::posix_time::milliseconds(m_sleepTime) );
            }
        }


    } else if( m_type == CONSUMER )
    {

        int tmp;

        while( true )
        {
            m_rQueue.pop(tmp);
            {
                boost::unique_lock<boost::mutex> lock(m_rLogMutex);
                std::cout << "[CONSUMER] : " << tmp << std::endl;
                boost::this_thread::sleep( boost::posix_time::milliseconds(m_sleepTime) );
            }
            if( tmp == 0 )
                break;
        }


    }

    {
        boost::unique_lock<boost::mutex> lock(m_rTasksMutex);
        --m_rTasks;
    }

    m_rCondClose.notify_one();
}