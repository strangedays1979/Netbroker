#pragma once

// std //
#include <string>

// System includes //
#include <boost/thread.hpp>

template<class T>
class SyncFifoQueue;

class Processor
{
public:

    enum Type{ PRODUCER, CONSUMER };

    Processor
        ( const std::string&            __rName
        , int&                          __rTasks
        , Type                          __type
        , SyncFifoQueue<int>&           __rQueue
        , boost::mutex&                 __rLogMutex
        , boost::mutex&                 __rTasksMutex
        , boost::condition_variable&    __rCondClose
        , const int&                    __rSleepTime
        , int                           __dataInts = 0
        );

    ~Processor();

    void job(void);

private:
    int&                        m_rTasks;
    boost::thread*              m_pThread;
    Type                        m_type;
    SyncFifoQueue<int>&         m_rQueue;
    int                         m_dataInts;
    boost::mutex&               m_rLogMutex;
    std::string                 m_name;
    boost::mutex&               m_rTasksMutex;
    boost::condition_variable&  m_rCondClose;
    int                         m_sleepTime;
};

