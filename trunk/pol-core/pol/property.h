/*
History
=======
2013/11/03 Nando: created

Notes
=======

*/
#ifndef H_PROPERTY_ACCESSOR
#define H_PROPERTY_ACCESSOR

#ifdef USE_SAFEINT
	#include "SafeInt.hpp"
#endif
/*
 Class for emulating getter/setter. By itself it's not very useful, but will return
 SafeInt<T> if USE_SAFEINT is defined. Can also be used for capping min/max values
 via constraints. Also helps normalizing the interface to public variables, so that later on
 we can change some of them (those which need updaters, for e.g.) to use getter/setters.

 If no constraints are used, has no impact on performance when compiled for release
 compared to using a public variable.

 Example:

   class X {
       ...
	   Property<u16> ar;
   }

   X obj;
   
   int y = obj.ar();   // gets the value
   obj.ar( 100 );      // sets the value

*/
template <class T, class constraint = NoConstraint>
class Property {
private:
	T _value;

public:
#ifdef USE_SAFEINT
	SafeInt<T> operator()() {
		return SafeInt<T>(_value);
	}
#else
	T operator()() {
		return _value;
	}
#endif

	T unsafe() {
		return _value;
	}

	void operator()(const T newValue) {
			_value = constraint::apply<T>(newValue);
	}
};


/*
 Constraints for Property class. Could be cleaner with some template
 metaprogramming, but this works for now. Essentially, each struct has
 a function apply<T>(x) which returns x in a valid range. To be used in 
 conjunction with Property<T, constraint = NoConstraint>.

*/

struct NoConstraint {	
	template<class T>	
	static T apply(const T x) {
		return x;
	}
};

template<int min> 
struct MinValue {
	template<class T>
	static T apply(const T x) {
		return (x < min) ? min : x;
	}
};

template<int max> 
struct MaxValue {
	template<class T>
	static T apply(const T x) {
		return (x > max) ? max : x;
	}
};

template<int min,int max>
struct MinMaxValue {
	template<class T>
	static T apply(const T x) {
		if (x < min)
			return min;
		else if (x > max)
			return max;
		else
			return x;
	}
};



/*
	 Preprocessor macro for defining get/set of dynamic properties. This requires the object
	 to have the functions this->getProp<T,key>() and this->setProp<T,key>(newValue).
 */
#define PROPERTY_MAP(propName,type,key) \
	type propName () {\
		return this->getmember<type,key>();\
	}\
	void propName (type tmp) {\
		this->setmember<type,key>(tmp);\
	}

#endif

#ifdef USE_SAFEINT
/* 
	The code below should go somewhere else. Right now (2013), property.h is the only thing to use SafeInts.
*/
struct PropertyHelper {
	static const char* stringFromError(SafeIntError x) {
		const int numberOfErrors = 3;
		const char* SafeIntErrorString[numberOfErrors] = {
			"No error",
			"Arithmetic overflow",
			"Divide by zero"
		};

		if (x < numberOfErrors) {
			return SafeIntErrorString[x];
		} else {
			return "";
		}
	}

};
#endif 