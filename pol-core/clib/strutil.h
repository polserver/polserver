/*
History
=======

Notes
=======

*/

#ifndef CLIB_STRUTIL
#define CLIB_STRUTIL

#include <string>

string hexint( unsigned short v );
string hexint( int v );
string hexint( unsigned v );
string hexint( long v );
string hexint( unsigned long v );

string decint( unsigned short v );
string decint( int v );
string decint( unsigned v );
string decint( long v );
string decint( unsigned long v );

void splitnamevalue( const string& istr, string& propname, string& propvalue );

void decodequotedstring( string& str );
void encodequotedstring( string& str );
string getencodedquotedstring( const string& in );

void mklower( string& str );
void mkupper( string& str );

string strlower( const string& str );
string strupper( const string& str );

#endif
