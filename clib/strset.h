/*
History
=======

Notes
=======

*/

#ifndef STRSET_H
#define STRSET_H

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include <set>
#include <string>

class StringSet
{
public:
    bool contains( const char* str ) const;
    void add( const char* str );
    void remove( const char* str );
    bool empty() const;

    void readfrom( const std::string& str );
    std::string extract() const;

    typedef std::set<std::string>::iterator iterator;
    iterator begin();
    iterator end();

    typedef std::set<std::string>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

private:
    std::set<std::string> strings_;
};

#endif
