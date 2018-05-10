


//=====================================================================================================//
/**
* @file         autoDeleter.hpp
* @brief        Proxy object for auto-deletion.
* @author       Filippo Spina
* @version
* @copyright    2012 Selex S.I.
* @date         09/04/2012
*/
//=====================================================================================================//




#pragma once


template<class T>
class AutoDeleter
{
public:
    AutoDeleter()
        : m_pObject(0)
    { }

    AutoDeleter(T*const _pObject)
        : m_pObject( _pObject )
        , pCnt ( new int(1) )
    { }

    AutoDeleter( const AutoDeleter<T>& _rRight )
        : m_pObject ( _rRight.m_pObject )
        , pCnt      ( _rRight.pCnt      )
    {
        ++(*pCnt);
    
    }

    ~AutoDeleter()
    {
        if(m_pObject && pCnt && (--(*pCnt)<=0 ) )
        {
            delete m_pObject;
            m_pObject = 0;

            delete pCnt;
            pCnt = 0;
        }
    }

    operator T*() const
    {
        return m_pObject;
    }

    operator T*&() const
    {
        return (T*&) m_pObject;
    }

    T* operator->() const
    {
        return m_pObject;
    }

    T* operator*() const
    {
        return m_pObject;
    }


    AutoDeleter<T>& operator=(T*const _pObject)
    {
        if(m_pObject && pCnt && (--(*pCnt)<=0 ) )
        {
            delete m_pObject;
            m_pObject = 0;

            delete pCnt;
            pCnt = 0;
        }

        if( !pCnt )
            pCnt = new int(1);

        m_pObject = _pObject;
        return *this;
    }

    AutoDeleter<T>& operator=( const AutoDeleter<T>& _rRight )
    {
        if(m_pObject && pCnt && (--(*pCnt)<=0 ) )
        {
            delete m_pObject;
            m_pObject = 0;

            delete pCnt;
            pCnt = 0;
        }

        pCnt = _rRight.pCnt;
        ++(*pCnt);

        m_pObject = _rRight.m_pObject;
        return *this;
    }

    T* get() const { return m_pObject; }

private:
    T *m_pObject;
    int* pCnt;

};