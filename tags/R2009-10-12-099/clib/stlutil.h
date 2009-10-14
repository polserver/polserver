/*
History
=======

Notes
=======

*/

#ifndef STLUTIL_H
#define STLUTIL_H

template <class C, class T, class A>
void ForEach( C& coll, void (*f)(T elem, A* staticdata), A* staticdata )
{
    typename C::iterator itr = coll.begin(), end = coll.end();
    for( ; itr != end; ++itr )
	{
		(*f)( *itr, staticdata );
	}
}
template <class C, class T>
void ForEach( C& coll, void (*f)(T elem) )
{
    typename C::iterator itr = coll.begin(), end = coll.end();
    for( ; itr != end; ++itr )
	{
		(*f)( *itr );
	}
}

template <class C, class T, class A>
void ConstForEach( C& coll, void (*f)(T elem, const A* staticdata), const A* staticdata )
{
    typename C::iterator itr = coll.begin(), end = coll.end();
    for( ; itr != end; ++itr )
	{
		(*f)( *itr, staticdata );
	}
}

template<class T>
void delete_all(T& coll)
{
    while (!coll.empty())
    {
        delete coll.back();
        coll.pop_back();
    }
}

template<class T,class A>
typename T::iterator find_in( T& coll, const A& value )
{
    typename T::iterator itr = coll.begin();
    typename T::iterator last = coll.end();

    while (itr != last && *itr != value) 
        ++itr;
  
    return itr;
}
template<class T,class A>
typename T::const_iterator const_find_in( const T& coll, const A& value )
{
    typename T::const_iterator itr = coll.begin();
    typename T::const_iterator last = coll.end();

    while (itr != last && *itr != value) 
        ++itr;
  
    return itr;
}
/*
works, but don't really want to include sstream, string
template<class V>
string make_string( V& x )
{
    ostringstream os;
    os << x;
    return os.str();
}
*/

template<class D, class S>    
inline D implicit_cast(const S& s) 
{
	return s; // fails unless S can be converted to D implicitly    
}

template<class D, class S>
inline D explicit_cast(const S& s)
{
    return static_cast<D>(s);
}

// GCC doesn't like this..sorry
//#define FOR_EACH(itr,Class,var) for( Class##::iterator itr = var##.begin(); itr != var##.end(); ++itr )

template<class S1, class S2>
inline int stringicmp( const S1& str1, const S2& str2 )
{
#ifdef _WIN32
    return stricmp( str1.c_str(), str2.c_str() );
#else
	return strcasecmp( str1.c_str(), str2.c_str() );
#endif
}
template<class S1, const char*>
inline int stringicmp( const S1& str1, const char* str2 )
{
#ifdef _WIN32
    return stricmp( str1.c_str(), str2 );
#else
	return strcasecmp( str1.c_str(), str2 );
#endif
}
template<const char*, class S2>
inline int stringicmp( const char* str1, const S2& str2 )
{
#ifdef _WIN32
    return stricmp( str1, str2.c_str() );
#else
	return strcasecmp( str1, str2.c_str() );
#endif
}

#if 0 //1 || defined(__GNUC__)
#   include <strstream>
#   include <string>
    class e_istringstream : public istrstream
    {
    public:
        e_istringstream( const string& str ) : istrstream( str.c_str(), str.length() ) {}
        e_istringstream( const char* s ) : istrstream( s ) {}
    };
    class e_ostringstream : public ostrstream
    {
    public:
        e_ostringstream() : ostrstream() {};
        string e_str() { string t = string(str(),pcount()); freeze(0); return t; }
        // void clear() { (*this) = e_ostringstream(); }
#if !defined(__GNUC__)
    private:
        using ostrstream::str;
#endif
    };
#else
#define ISTRINGSTREAM istringstream

#define OSTRINGSTREAM ostringstream
#define OSTRINGSTREAM_STR(x) x.str()
/*#   include <sstream>
    class e_istringstream : public istringstream
    {
    public:
        explicit e_istringstream( const string& str ) : istringstream( str ) {}
    };
    class e_ostringstream : public ostringstream
    {
    public:
        e_ostringstream() : ostringstream() {};
        string e_str() { return str(); }
        // void clear() { str(""); }
    };
*/
#endif

template<class T>
string tostring( const T& v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}

#endif
