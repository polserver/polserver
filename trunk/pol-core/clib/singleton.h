/*
History
=======

Notes
=======

*/

#ifndef CLIB_SINGLETON_H
#define CLIB_SINGLETON_H
#include "../pol/polsem.h"

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
				PolLock lck; //critical double check
				if (!_instance)
				{
					_instance = new T();
					atexit(&Singleton<T>::_cleanup);
				}
			}
			return _instance;
		}
	private:
		static T* _instance;
	protected:
		Singleton() { }
		~Singleton() { }
	private: // Forbid copies
		Singleton( Singleton& ); 
		Singleton& operator=( Singleton& );
};
template <typename T> T* Singleton <T>::_instance = NULL;

#endif

