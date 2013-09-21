/*
History
=======
2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no sense.

Notes
=======

*/

#include "stl_inc.h"

#include "stlutil.h"
#include "strutil.h"
#include "unittest.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4244 )
#endif

string hexint( unsigned short v )
{
    OSTRINGSTREAM os;
    os << "0x" << hex << v;
    return OSTRINGSTREAM_STR(os);
}

string hexint( signed int v )
{
    OSTRINGSTREAM os;
    os << "0x" << hex << v;
    return OSTRINGSTREAM_STR(os);
}
string hexint( unsigned int v )
{
    OSTRINGSTREAM os;
    os << "0x" << hex << v;
    return OSTRINGSTREAM_STR(os);
}

string hexint( signed long v )
{
    OSTRINGSTREAM os;
    os << "0x" << hex << v;
    return OSTRINGSTREAM_STR(os);
}
string hexint( unsigned long v )
{
    OSTRINGSTREAM os;
    os << "0x" << hex << v;
    return OSTRINGSTREAM_STR(os);
}
#ifdef _WIN64
string hexint( size_t v )
{
	OSTRINGSTREAM os;
	os << "0x" << hex << v;
	return OSTRINGSTREAM_STR(os);
}
#endif
string decint( unsigned short v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}

string decint( signed int v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}

string decint( unsigned int v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}

string decint( signed long v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}
string decint( unsigned long v )
{
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR(os);
}

#ifdef _WIN64
string decint( size_t v )
{
	OSTRINGSTREAM os;
	os << v;
	return OSTRINGSTREAM_STR(os);
}
#endif
void splitnamevalue( const string& istr, 
                     string& propname, 
                     string& propvalue )
{
    string::size_type start = istr.find_first_not_of( " \t\r\n" );
    if (start != string::npos)
    {
        string::size_type delimpos = istr.find_first_of( " \t\r\n=", start+1 );
        if (delimpos != string::npos)
        {
            string::size_type valuestart = istr.find_first_not_of( " \t\r\n", delimpos+1 );
            string::size_type valueend = istr.find_last_not_of( " \t\r\n" );
            propname = istr.substr( start, delimpos-start );
            if (valuestart != string::npos && valueend != string::npos)
            {
                propvalue = istr.substr( valuestart, valueend-valuestart+1 );
            }
            else
            {
                propvalue = "";
            }
        }
        else
        {
            propname = istr.substr( start, string::npos );
            propvalue = "";
        }
    }
    else
    {
        propname = "";
        propvalue = "";
    }
}

void test_splitnamevalue( const string& istr, const string& exp_pn, const string& exp_pv )
{
    string pn, pv;
    splitnamevalue( istr, pn, pv );
    if (pn != exp_pn || pv != exp_pv)
    {
        cout << "splitnamevalue( \"" << istr << "\" ) fails!" << endl;
    }
}

void test_splitnamevalue()
{
    test_splitnamevalue( "a b", "a", "b" );
    test_splitnamevalue( "av bx", "av", "bx" );
    test_splitnamevalue( "nm=valu", "nm", "valu" );
    test_splitnamevalue( "nm:valu", "nm:valu", "" );
    test_splitnamevalue( "nm", "nm", "" );
    test_splitnamevalue( "  nm", "nm", "" );
    test_splitnamevalue( "  nm  ", "nm", "" );
    test_splitnamevalue( "  nm valu", "nm", "valu" );
    test_splitnamevalue( "  nm   value   ", "nm", "value" );
    test_splitnamevalue( "  nm  value is multiple words", "nm", "value is multiple words" );
    test_splitnamevalue( "  nm  value is multiple words\t ", "nm", "value is multiple words" );
}
UnitTest test_splitnamevalue_obj( test_splitnamevalue );

void decodequotedstring( string& str )
{
    string tmp;
    tmp.swap( str );
    const char* s = tmp.c_str();
    str.reserve( tmp.size() );
    ++s;
    while (*s)
    {
        char ch = *s++;

        switch( ch )
        {
        case '\\':
            ch = *s++;
            switch( ch )
            {
            case '\0':
                return;
            case 'n':               // newline
                str += "\n";
                break;
            default:                // slash, quote, etc
                str += ch;
                break;
            }
            break;
        
        case '\"':
            return;

        default:
            str += ch;
            break;
        }
    }
}
void encodequotedstring( string& str )
{
    string tmp;
    tmp.swap( str );
    const char* s = tmp.c_str();
    str.reserve( tmp.size()+2 );
    str += "\"";

    while (*s)
    {
        char ch = *s++;
        switch( ch )
        {
        case '\\':
            str += "\\\\";
            break;
        case '\"':
            str += "\\\"";
            break;
        case '\n':
            str += "\\n";
            break;
        default:
            str += ch;
            break;
        }
    }
    
    str += "\"";
}
string getencodedquotedstring( const string& in )
{
    string tmp = in;
    encodequotedstring(tmp);
    return tmp;
}
void test_dqs( const string& in, const string& out )
{
    string tmp = in;
    decodequotedstring(tmp);
    if (tmp != out)
    {
        cout << "decodequotedstring( " << in << " ) fails!" << endl;
    }
    encodequotedstring(tmp);
    if (tmp != in)
    {
        cout << "encodequotedstring( " << out << " ) fails!" << endl;
    }
}

void test_convertquotedstring()
{
    test_dqs( "\"hi\"", "hi" );
    test_dqs( "\"hi \"", "hi " );
    test_dqs( "\" hi \"", " hi " );
    test_dqs( "\" \\\"hi\"", " \"hi" );
}
UnitTest test_convertquotedstring_obj(test_convertquotedstring);


void mklower( string& str )
{
    for(auto &elem : str)
    {
        elem = tolower(elem);
    }
}

void mkupper( string& str )
{
    for(auto &elem : str)
    {
        elem = toupper(elem);
    }
}

string strlower( const string& str )
{
    string tmp = str;
    mklower(tmp);
    return tmp;
}

string strupper( const string& str )
{
    string tmp = str;
    mkupper(tmp);
    return tmp;
}
