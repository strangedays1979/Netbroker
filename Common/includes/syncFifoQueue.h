

//=====================================================================================================//
/**
* @file         syncFifoQueue.h
* @brief        Boost based synchronized blocking/non-blocking queue.
* @author       Filippo Spina
* @version
* @copyright    2012 Selex S.I.
* @date         09/11/2010
*/
//=====================================================================================================//



#pragma once

// STD includes //
#include <deque>

// Boost thread includes //
#include <boost/thread.hpp>

/**
@brief Blocking FIFO queue.
**/
template<class T>
class SyncFifoQueue: private std::deque<T>
{
public:
    SyncFifoQueue(const size_t& _maxSize );

    void push(const T& _data );
    void pop (T& _data );
    bool try_pop(T& _data );
    bool timed_pop(T& _data, const boost::posix_time::milliseconds& __rTimer);
    bool async_pop(T& _data );
    size_t size();

    void signalForKill();
    void reset();

    bool isFull();

private:
    bool                        m_sigKill;
    const size_t                m_maxSize;
    size_t                      m_currSize;
    boost::condition_variable   m_cond;
    boost::try_mutex            m_mtx;
};

template<class T>
void SyncFifoQueue<T>::signalForKill()
{
    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    m_sigKill = true;
    m_cond.notify_all();

}

template<class T>
void SyncFifoQueue<T>::reset()
{
    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    std::deque<T>::clear();
    m_sigKill       = false;
    m_currSize      = 0;
}


template<class T>
SyncFifoQueue<T>::SyncFifoQueue(const size_t& _maxSize )
: m_maxSize     ( _maxSize  )
, m_currSize    ( 0         )
, m_sigKill     ( false     )
{






}

template<class T>
bool SyncFifoQueue<T>::isFull()
{
    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    return m_maxSize == m_currSize;

}


template<class T>
void SyncFifoQueue<T>::push(const T& _data )
{

    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    while( !m_sigKill && ( m_currSize >= m_maxSize) )
        m_cond.wait(lock);

    switch( m_sigKill )
    {
    case true:
        return;
    default:
        ;
    }

    push_back( _data );
    ++m_currSize;
    m_cond.notify_all();

}


template<class T>
void SyncFifoQueue<T>::pop(T& _data )
{

    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    while( !m_sigKill && (m_currSize <= 0) )
        m_cond.wait(lock);

    switch( m_sigKill )
    {
    case true:
        return;
    default:
        ;
    }

    _data = front();
    pop_front();
    --m_currSize;
    m_cond.notify_all();
}

template<class T>
bool SyncFifoQueue<T>::timed_pop(T& _data, const boost::posix_time::milliseconds& __rTimer)
{


    boost::unique_lock<boost::try_mutex> lock(m_mtx);

    while( !m_sigKill &&  (m_currSize <= 0)  )
        if( !m_cond.timed_wait(lock, __rTimer) )
            return false;

    switch( m_sigKill )
    {
    case true:
        return false;
    default:
        ;
    }

    _data = front();
    pop_front();
    --m_currSize;
    m_cond.notify_all();


    return true;

}

template<class T>
bool SyncFifoQueue<T>::try_pop(T& _data )
{

    boost::try_mutex::scoped_try_lock  lock(m_mtx);

    if (!lock.owns_lock())
        return false;


    while( !m_sigKill && (m_currSize <= 0 ) )
        return false;

    switch( m_sigKill )
    {
    case true:
        return false;
    default:
        ;
    }

    _data = front();
    pop_front();
    --m_currSize;
    m_cond.notify_all();
    return true;

}


template<class T>
bool SyncFifoQueue<T>::async_pop(T& _data )
{
    boost::unique_lock<boost::try_mutex> lock(m_mtx);
    while( !m_sigKill &&  (m_currSize <= 0) )
        return false;

    switch( m_sigKill )
    {
    case true:
        return false;
    default:
        ;
    }

    _data = front();
    pop_front();
    --m_currSize;
    m_cond.notify_all();
    return true;

}


template<class T>
size_t  SyncFifoQueue<T>::size()
{

    size_t retVal =0;

    {
        boost::unique_lock<boost::try_mutex> lock(m_mtx);
        retVal = std::deque<T>::size();
    }

    return retVal;

}