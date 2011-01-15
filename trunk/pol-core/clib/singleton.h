/*
History
=======

Notes
=======

*/

#ifndef CLIB_SINGLETON_H
#define CLIB_SINGLETON_H
#include "hbmutex.h"

template <typename T>
class Singleton
{
	private:
		static void _cleanup() {delete _instance; _instance = NULL;}
	public:
		static T* instance()
		{
			if (!_instance)
			{
				singleton_mutex.lock(); //critical double check
				if (!_instance)
				{
					_instance = new T();
					atexit(&Singleton<T>::_cleanup);
				}
				singleton_mutex.unlock();
			}
			return _instance;
		}
	private:
		static T* _instance;
		static Mutex singleton_mutex;
	protected:
		Singleton() { }
		~Singleton() { }
	private: // Forbid copies
		Singleton( Singleton& ); 
		Singleton& operator=( Singleton& );
};
template <typename T> T* Singleton <T>::_instance = NULL;
template <typename T> Mutex Singleton <T>::singleton_mutex;

#endif

