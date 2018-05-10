#ifndef _NO_LOCK_POOL
#define _NO_LOCK_POOL

#include <deque>
#include "BDS.h"

namespace TDS
{
template<typename T> class NoLockPool
{
private:
	int MaxSize;
	int TrueSize;
	int Incr;
	std::deque<T*> Pool;
	std::deque<T*> Allocation;
	void *RelQueue; // SPQ

private:
	bool grow();

public:
	NoLockPool ( const int MaxSize, const int Incr = 0 );
	~NoLockPool();
	T* get();
	void release ( T* Item );
	void recycle ( T* Item );
};

template<typename T>
bool NoLockPool<T>::grow()
{
	int MyIncr = this->MaxSize - this->TrueSize;
	if ( MyIncr <= 0 ) return false;
	MyIncr = (MyIncr > this->Incr) ? this->Incr : MyIncr;

	T* MyPtr = new T[MyIncr];
	this->Allocation.push_front ( MyPtr );
	for ( int iii = 0; iii < MyIncr; iii++ )
	{
		this->Pool.push_back ( &MyPtr[iii] );
	}
	this->TrueSize += MyIncr;
	return true;
}

template<typename T>
NoLockPool<T>::NoLockPool ( const int MaxSize, const int Incr )
{
	this->MaxSize = MaxSize;
	this->TrueSize = 0;

	this->Incr = Incr;
	this->Incr = (Incr > this->MaxSize) ? this->MaxSize : Incr;

	int MinIncr = (this->MaxSize + 9) / 10;
	this->Incr = (this->Incr > MinIncr) ? this->Incr : MinIncr;

	BDS::SPQ_Create ( this->MaxSize, 1, &this->RelQueue );

	this->grow();
}

template<typename T>
NoLockPool<T>::~NoLockPool()
{
	BDS::SPQ_Remove ( this->RelQueue );

	this->Pool.clear();

	while ( false == this->Allocation.empty() )
	{
		T* MyPtr = this->Allocation.front();
		delete[] MyPtr;
		this->Allocation.pop_front();
	}
}

template<typename T>
T* NoLockPool<T>::get()
{
	T* MyPtr;

	if ( true == this->Pool.empty() )
	{
		//printf ( "NoLockPool::get(RelQueue): %d %d\n", this->TrueSize, this->Pool.size() ); fflush(stdout);
		if ( BDS::SPQ_OK == BDS::SPQ_GetHead ( this->RelQueue, (void **)&MyPtr ) ) return MyPtr;

		if ( false == this->grow() ) return NULL;
	}

	//printf ( "NoLockPool::get(Pool): %d %d\n", this->TrueSize, this->Pool.size() ); fflush(stdout);
	MyPtr = this->Pool.front();
	this->Pool.pop_front();
	return MyPtr;
}

template<typename T>
void NoLockPool<T>::release ( T* Item )
{
	//printf ( "NoLockPool::release(RelQueue): %d %d\n", this->TrueSize, this->Pool.size() ); fflush(stdout);
	BDS::SPQ_PutTail ( this->RelQueue, (void *)Item );
	return;
}

template<typename T>
void NoLockPool<T>::recycle ( T* Item )
{
	//printf ( "NoLockPool::recycle(Pool): %d %d\n", this->TrueSize, this->Pool.size() ); fflush(stdout);
	this->Pool.push_back ( Item );
	return;
}

};

#endif // _NO_LOCK_POOL

