/*
History
=======

Notes
=======

*/

#ifndef CLIB_STRUTIL
#define CLIB_STRUTIL

#include <string>

std::string hexint( unsigned short v );
std::string hexint( signed int v );
std::string hexint( unsigned int v );
std::string hexint( signed long v );
std::string hexint( unsigned long v );

std::string decint( unsigned short v );
std::string decint( signed int v );
std::string decint( unsigned int v );
std::string decint( signed long v );
std::string decint( unsigned long v );

void splitnamevalue( const std::string& istr,std:: string& propname, std::string& propvalue );

void decodequotedstring( std::string& str );
void encodequotedstring( std::string& str );
string getencodedquotedstring( const std::string& in );

void mklower( std::string& str );
void mkupper( std::string& str );

std::string strlower( const std::string& str );
std::string strupper( const std::string& str );

#endif
