#pragma once


#include <boost/thread.hpp>
#include <vector>

class Group;
class IDispatcher;

class DispatcherTester
{

public:

    enum Modality{ PRODUCER, CONSUMER };




    DispatcherTester( IDispatcher*& _disp
        , Modality _mod
        , boost::mutex* __rLogMutex
        , std::vector<Group> _groups
        , boost::mutex& __rTasksMutex
        , boost::condition_variable& __rCondClose
        , int& _tasks);

    ~DispatcherTester();

    void job(void);

    void sendToDispatcher();

    void receiveFromDispatcher();

    void start();

private:
    Modality m_mod;
    boost::thread* m_pThread;
    boost::mutex* m_pLogMutex;

    IDispatcher*& m_pDispatcher;

    std::vector<Group> m_groups;

    boost::mutex& m_rTasksMutex;
    boost::condition_variable& m_rCondClose;
    int& m_rTasks;

};