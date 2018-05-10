#ifndef _SYNCH_QUEUE
#define _SYNCH_QUEUE

#include <deque>
#include <vector>
#include "BDS.h"
#include "SDS.h"
#include "OSAL.h"

namespace TDS
{
enum
{
	MIN_SPARE_ITEMS = 3
};
/*
La classe "T" deve implementare il metodo "flush()"
che viene invocato quando si suota la coda
*/
template<typename T> class SynchQueue
{
private:
	int MaxSize;
	QUEUE::SynchQueue ItemQueue;
	void *FreeItem; // SPQ
	T* Item;
	int SpareItems;

	struct
	{
		bool Used;
		// TBC: Sostituzione con primitive "Boost" 
		OSAL::mutex Mutex;
	} MyLock;

	void Lock()
	{
		if ( true == this->MyLock.Used ) OSAL::MutexLock ( &this->MyLock.Mutex );
	};

	void UnLock()
	{
		if ( true == this->MyLock.Used ) OSAL::MutexUnlock ( &this->MyLock.Mutex );
	};

public:
	SynchQueue ( const int MaxSize, bool Lock = false );
	SynchQueue ( const int MaxSize, const int Spare, bool Lock = false );
	~SynchQueue();

	void Empty();

	void Items ( std::vector<T*> &V );

	T* GetFreeItem();
	void ReleaseFreeItem ( T* Item );

	QUEUE::Result Get ( T* &Item );
	QUEUE::Result GetWait ( T* &Item );
	QUEUE::Result Put ( T* Item );
	QUEUE::Result PutWait ( T* Item );
	void ForceTrap();
	void ResetTrap();
	void WaitForTrapReset();
	bool CheckForTrap();
};

template<typename T>
SynchQueue<T>::SynchQueue ( const int MaxSize, bool Lock ) : MaxSize(MaxSize), ItemQueue(MaxSize)
{
	this->SpareItems = MIN_SPARE_ITEMS;
	BDS::SPQ_Create ( this->MaxSize + this->SpareItems, 1, &this->FreeItem );

	this->Item = new T[this->MaxSize + this->SpareItems];
	this->ItemQueue.ForceTrap();
	this->Empty();
	this->ItemQueue.ResetTrap();

	this->MyLock.Used = Lock;
	if ( true == this->MyLock.Used )
	{
		OSAL::MutexCreate ( &this->MyLock.Mutex );
	}
}

template<typename T>
SynchQueue<T>::SynchQueue ( const int MaxSize, const int Spare, bool Lock ) : MaxSize(MaxSize), ItemQueue(MaxSize)
{
	this->SpareItems = MIN_SPARE_ITEMS;
	if ( this->SpareItems < Spare ) this->SpareItems = Spare;
	BDS::SPQ_Create ( this->MaxSize + this->SpareItems, 1, &this->FreeItem );

	this->Item = new T[this->MaxSize + this->SpareItems];
	this->ItemQueue.ForceTrap();
	this->Empty();
	this->ItemQueue.ResetTrap();

	this->MyLock.Used = Lock;
	if ( true == this->MyLock.Used )
	{
		OSAL::MutexCreate ( &this->MyLock.Mutex );
	}
}

template<typename T>
SynchQueue<T>::~SynchQueue()
{
	delete[] this->Item;
	BDS::SPQ_Remove ( this->FreeItem );
	if ( true == this->MyLock.Used )
	{
		OSAL::MutexDelete ( &this->MyLock.Mutex );
	}
}

template<typename T>
void SynchQueue<T>::Empty()
{
	this->ItemQueue.Empty();

	this->Lock();
	void *DummyPtr;
	while ( BDS::SPQ_OK == BDS::SPQ_GetHead ( this->FreeItem, &DummyPtr ) ) {}

	for ( int iii = 0; iii < (this->MaxSize + this->SpareItems); iii++ )
	{
		this->Item[iii].flush();
		BDS::SPQ_PutTail ( this->FreeItem, (void *)&Item[iii] );
	}
	this->UnLock();
}

template<typename T>
void SynchQueue<T>::Items ( std::vector<T*> &V )
{
	V.clear();
	this->Lock();
	for ( int iii = 0; iii < (this->MaxSize + this->SpareItems); iii++ )
	{
		V.push_back ( &this->Item[iii] );
	}
	this->UnLock();
}

template<typename T>
T* SynchQueue<T>::GetFreeItem()
{
	this->Lock();
	T* MyPtr;
	BDS::SPQ_GetHead ( this->FreeItem, (void **)&MyPtr );
	this->UnLock();
	return MyPtr;
}

template<typename T>
void SynchQueue<T>::ReleaseFreeItem ( T* Item )
{
	this->Lock();
	BDS::SPQ_PutTail ( this->FreeItem, (void *)Item );
	this->UnLock();
}

template<typename T>
QUEUE::Result SynchQueue<T>::Get ( T* &Item )
{
	QUEUE::Result Res = this->ItemQueue.Get ( (void **)&Item );
	return Res;
}

template<typename T>
QUEUE::Result SynchQueue<T>::GetWait ( T* &Item )
{
	QUEUE::Result Res = this->ItemQueue.GetWait ( (void **)&Item );
	return Res;
}

template<typename T>
QUEUE::Result SynchQueue<T>::Put ( T* Item )
{
	return this->ItemQueue.Put ( (void *)Item );
}

template<typename T>
QUEUE::Result SynchQueue<T>::PutWait ( T* Item )
{
	return this->ItemQueue.PutWait ( (void *)Item );
}

template<typename T>
void SynchQueue<T>::ForceTrap()
{
	this->ItemQueue.ForceTrap();
}

template<typename T>
void SynchQueue<T>::ResetTrap()
{
	this->ItemQueue.ResetTrap();
}

template<typename T>
void SynchQueue<T>::WaitForTrapReset()
{
	this->ItemQueue.WaitForTrapReset();
}

template<typename T>
bool SynchQueue<T>::CheckForTrap()
{
	return this->ItemQueue.CheckForTrap();
}

};

#endif // _SYNCH_QUEUE