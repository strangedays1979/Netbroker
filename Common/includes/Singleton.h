
//=====================================================================================================//
/**
* @file         Singleton.h
* @brief        Templated Singleton
* @author       Filippo Spina
* @version
* @copyright    2012 Selex S.I.
* @date         09/11/2010
*/
//=====================================================================================================//

#ifndef __WIDTOOLS__SINGLETON__
#define __WIDTOOLS__SINGLETON__

template<class T>
class Singleton
{

protected:
	Singleton();
	virtual ~Singleton();

public:


	static T* getPtr();


private:
	static T* pData;
};


template<class T>
Singleton<T>::Singleton()
{

}

template<class T>
Singleton<T>::~Singleton()
{


}

template<class T>
T* Singleton<T>::getPtr()
{
	if( !pData )
		pData = new T;

	return pData;
}

template<class T>
T* Singleton<T>::pData = 0;


#endif // __WIDTOOLS__SINGLETON__

