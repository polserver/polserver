/*
History
=======
2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include "../clib/cfgelem.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "proplist.h"

#define pf_endl '\n'

PropertyList::PropertyList()
{
}

PropertyList::PropertyList( const PropertyList& props)  //dave added 1/26/3
{
	copyprops(props);
}


bool PropertyList::getprop( const string& propname, string& propval ) const
{
    Properties::const_iterator itr = properties.find( propname );
    if (itr == properties.end())
    {
        return false;
    }
    else
    {
        propval = (*itr).second;
        return true;
    }
}
void PropertyList::setprop( const std::string& propname, const std::string& propvalue )
{
    properties[ propname ] = propvalue;
}

void PropertyList::eraseprop( const std::string& propname )
{
    properties.erase( propname );
}

void PropertyList::copyprops( const PropertyList& from )
{
	//dave 4/25/3 map insert won't overwrite with new values, so remove those first and then reinsert.
	Properties::const_iterator itr;
	for(itr = from.properties.begin(); itr != from.properties.end(); ++itr)
		properties.erase(itr->first);

    properties.insert( from.properties.begin(), from.properties.end() );
}

void PropertyList::clear()
{
    properties.clear();
}

void PropertyList::getpropnames( std::vector< std::string >& propnames ) const
{
    for( Properties::const_iterator itr = properties.begin(); itr != properties.end(); ++itr )
    {
        const string& first = (*itr).first;
        propnames.push_back( first );
    }
}

void PropertyList::printProperties( ostream& os ) const
{   
    for( Properties::const_iterator itr = properties.begin(); itr != properties.end(); ++itr )
    {
        const string& first = (*itr).first;
        if (first[0] != '#')
        {
            os << "\tCProp\t" << first << " " << (*itr).second << pf_endl;
        }
    }
}
void PropertyList::printProperties( ConfigElem& elem ) const
{   
    for( Properties::const_iterator itr = properties.begin(); itr != properties.end(); ++itr )
    {
        const string& first = (*itr).first;
        if (first[0] != '#')
        {
            elem.add_prop( "CProp", (first + "\t" + (*itr).second).c_str() );
        }
    }
}
void PropertyList::printPropertiesAsStrings( ostream& os ) const
{   
    for( Properties::const_iterator itr = properties.begin(); itr != properties.end(); ++itr )
    {
        const string& first = (*itr).first;
        if (first[0] != '#')
        {
            os << "\t" << first << " " << (*itr).second << pf_endl;
        }
    }
}

void PropertyList::readProperties( ConfigElem& elem )
{
    string tempstr;
    while (elem.remove_prop( "StrProp", &tempstr ))
    {
        string propname;
        string propvalue;

        splitnamevalue( tempstr, propname, propvalue );

        setprop( propname, "s"+propvalue );
    }
    while (elem.remove_prop( "CProp", &tempstr ))
    {
        string propname;
        string propvalue;

        splitnamevalue( tempstr, propname, propvalue );

        setprop( propname, propvalue );
    }
}

void PropertyList::readRemainingPropertiesAsStrings( ConfigElem& elem )
{
    string propname, propvalue;
    while (elem.remove_first_prop( &propname, &propvalue ))
    {
        setprop( propname, propvalue );
    }
}

/*
void PropertyList::magicReadProperties( ConfigElem& elem )
{
    string propname, propval;

    while (elem.remove_first_prop( &propname, &propval ))
    {
        const char* t_real;
        const char* t_long;
        if
            strto
        char ch = propval[0];
        if (isxdigit(ch) || ch == '-' || ch == '+')
    }
}
*/

bool PropertyList::operator==(const PropertyList& plist) const
{
	return (this->properties == plist.properties);
}

PropertyList& PropertyList::operator-( const std::set<std::string>& CPropNames) //dave added 1/26/3
{
	std::set<std::string>::const_iterator itr;
	for(itr = CPropNames.begin(); itr != CPropNames.end(); ++itr)
	{
		eraseprop(*itr);
	}

	return *this;
}

void PropertyList::operator-=( const std::set<std::string>& CPropNames)  //dave added 1/26/3
{
	std::set<std::string>::const_iterator itr;
	for(itr = CPropNames.begin(); itr != CPropNames.end(); ++itr)
	{
		eraseprop(*itr);
	}
}

