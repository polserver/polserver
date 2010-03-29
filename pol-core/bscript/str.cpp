/*
History
=======
2007/12/09 Shinigami: removed ( is.peek() != EOF ) check from String::unpackWithLen()
                      will not work with Strings in Arrays, Dicts, etc.
2008/02/08 Turley:    String::unpackWithLen() will accept zero length Strings
2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no sense.

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdlib.h>
#include <string.h>

#ifdef __GNUG__
#	include <streambuf>
#endif

#include "berror.h"
#include "bobject.h"
#include "impstr.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4244 )
#endif

String::String(BObjectImp& objimp) : BObjectImp( OTString )
{
    value_ = objimp.getStringRep();
}

String::String(const char *s, int len) :
    BObjectImp( OTString ),
    value_( s, len )
{
}

String *String::StrStr(int begin, int len)
{
	return new String( value_.substr( begin-1, len ) );
}

String *String::ETrim(const char* CRSet, int type)
{
	string tmp = value_;

	if ( type == 1 ) // This is for Leading Only.
	{
		// Find the first character position after excluding leading blank spaces 
		size_t startpos = tmp.find_first_not_of( CRSet );
		if( string::npos != startpos )
			tmp = tmp.substr( startpos );
		else
			tmp = "";
		return new String( tmp );
	}
	else if ( type == 2 ) // This is for Trailing Only.
	{
		// Find the first character position from reverse 
		size_t endpos = tmp.find_last_not_of( CRSet );
		if( string::npos != endpos ) 
			tmp = tmp.substr( 0, endpos+1 ); 
		else
			tmp = "";
		return new String( tmp );
	}
	else if ( type == 3 )
	{
		// Find the first character position after excluding leading blank spaces  
		size_t startpos = tmp.find_first_not_of( CRSet );
		// Find the first character position from reverse af  
		size_t endpos = tmp.find_last_not_of( CRSet );

		// if all spaces or empty return on empty string  
		if(( string::npos == startpos ) || ( string::npos == endpos))  
			tmp = "";
		else  
			tmp = tmp.substr( startpos, endpos-startpos+1 );
		return new String( tmp );
	}
	else
		return new String( tmp );
}

void String::EStrReplace(String* str1, String* str2)
{
	string::size_type valpos;
	while ( string::npos != (valpos = value_.find(str1->value_)) )
    {
		value_.replace( valpos, str1->length(), str2->value_ );
    }
}

void String::ESubStrReplace(String* replace_with, unsigned int index, unsigned int len)
{
	value_.replace(index-1, len, replace_with->value_);
}

string String::pack() const
{
    return "s" + value_;
}

void String::packonto( ostream& os ) const
{
    os << "S" << value_.size() << ":" << value_;
}
void String::packonto( ostream& os, const string& value )
{
    os << "S" << value.size() << ":" << value;
}

BObjectImp* String::unpack( const char* pstr )
{
    return new String( pstr );
}

BObjectImp* String::unpack( istream& is )
{
    string tmp;
    getline( is, tmp );

    return new String( tmp );
}

BObjectImp* String::unpackWithLen( istream& is )
{
    unsigned len;
	char colon; 
    if (! (is >> len >> colon))
    {
        return new BError( "Unable to unpack string length." );
    }
	if ( (int)len < 0 )
    {
		return new BError( "Unable to unpack string length. Invalid length!" );
    }
	if ( colon != ':' ) 
	{ 
		return new BError( "Unable to unpack string length. Bad format. Colon not found!" ); 
	}

    is.unsetf( ios::skipws );
    string tmp;
    tmp.reserve( len );
    while (len--)
    {
		char ch = '\0'; 
		if(!(is >> ch) || ch == '\0') 
		{ 
			return new BError( "Unable to unpack string length. String length excessive."); 
		}
		tmp += ch;
    }

	is.setf( ios::skipws );
    return new String( tmp );
}

unsigned int String::sizeEstimate() const
{
    return sizeof(String) + value_.capacity();
}

/*
	0-based string find
	find( "str srch", 2, "srch"):
    01^-- start
*/
int String::find(int begin, const char *target)
{
    // TODO: check what happens in string if begin position is out of range
    string::size_type pos;
    pos = value_.find( target, begin );
    if (pos == string::npos)
        return -1;
    else
        return pos;
}

// Returns the amount of alpha-numeric characters in string.
unsigned int String::alnumlen(void) const
{
	unsigned int c=0;
	while (isalnum(value_[c]))
	{
		c++;
	}
	return c;
}

unsigned int String::SafeCharAmt() const
{
	int strlen = this->length();
	for ( int i=0; i < strlen; i++ )
	{
		char tmp = value_[i];
		if ( isalnum(tmp) ) // a-z A-Z 0-9
			continue;
		else if ( ispunct(tmp) ) // !"#$%&'()*+,-./:;<=>?@{|}~
		{
			if ( tmp == '{' || tmp == '}' )
				return i;
			else
				continue;
		}
		else
		{
			return i;
		}
	}
	return strlen;
}

void String::reverse(void)
{
    ::reverse( value_.begin(), value_.end() );
}

void String::set( char *newstr )
{
    value_ = newstr;
    delete newstr;
}


BObjectImp* String::selfPlusObjImp(const BObjectImp& objimp) const
{
    return new String( value_ + objimp.getStringRep() );
}

void String::remove(const char *rm)
{
    int len = strlen(rm);
    string::size_type pos = value_.find( rm );
    if (pos != string::npos)
        value_.erase( pos, len );
}

BObjectImp* String::selfMinusObjImp(const BObjectImp& objimp) const
{
    String *tmp = (String *) copy();
    if (objimp.isa( OTString )) 
    {
        const String& to_remove = (String&) objimp;
        tmp->remove(to_remove.value_.data());
    } 
    else
    {
        tmp->remove( objimp.getStringRep().data() );
    }
    return tmp;
}


bool String::isEqual(const BObjectImp& objimp) const
{
    if (objimp.isa( OTString ))
    {
        return ( value_ == static_cast<const String&>(objimp).value_ );
    }
    else
    {
        return false;
    }
}

/* since non-Strings show up here as not equal, we make them less than. */
bool String::isLessThan(const BObjectImp& objimp) const
{
    if (objimp.isa( OTString ))
    {
        return ( value_ < static_cast<const String&>(objimp).value_ );
    }
    else if (objimp.isa( OTUninit ) || objimp.isa( OTError ))
    {
        return false;
    }
    else
    {
        return true;
    }
}

String *String::midstring(int begin, int len) const
{
    return new String( value_.substr( begin-1, len ) );
}

void String::toUpper( void )
{
    for( string::iterator itr = value_.begin(); itr != value_.end(); ++itr )
    {
        *itr = toupper(*itr);
    }
}

void String::toLower( void )
{
    for( string::iterator itr = value_.begin(); itr != value_.end(); ++itr )
    {
        *itr = tolower(*itr);
    }
}

BObjectImp* String::array_assign( BObjectImp* idx, BObjectImp* target )
{
    string::size_type pos, len;

    // first, determine position and length.
    if (idx->isa( OTString ))
    {
		String& rtstr = (String&) *idx;
        pos = value_.find( rtstr.value_ );
        len = rtstr.length();
    }
    else if (idx->isa( OTLong ))
    {
        BLong& lng = (BLong&) *idx;
        pos = lng.value()-1;
        len = 1;
    }
    else if (idx->isa( OTDouble ))
    {
        Double& dbl = (Double&)*idx;
        pos = static_cast<string::size_type>(dbl.value());
        len = 1;
    }
    else
    {
        return UninitObject::create();
    }

    if (pos != string::npos)
    {
        if (target->isa( OTString ))
        {
            String* target_str = (String*) target;
            value_.replace( pos, len, target_str->value_ );
        }
        return this;
    }
    else
    {
        return UninitObject::create();
    }
}

BObjectRef String::OperMultiSubscriptAssign( stack<BObjectRef>& indices, BObjectImp* target )
{
    BObjectRef start_ref = indices.top();
    indices.pop();
    BObjectRef length_ref = indices.top();
    indices.pop();

    BObject& length_obj = *length_ref;
    BObject& start_obj = *start_ref;

    BObjectImp& length = length_obj.impref();
    BObjectImp& start = start_obj.impref();

    // first deal with the start position.
    unsigned index;
    if (start.isa( OTLong ))
    {
        BLong& lng = (BLong&) start;
        index = (unsigned) lng.value();
        if (index == 0 || index > value_.size())
            return BObjectRef(new BError( "Subscript out of range" ));

    }
    else if (start.isa( OTString ))
    {
		String& rtstr = (String&) start;
        string::size_type pos = value_.find( rtstr.value_ );
        if (pos != string::npos)
            index = pos+1;
        else
            return BObjectRef(new UninitObject);
    }
    else
    {
        return BObjectRef(copy());
    }

    // now deal with the length.
    int len;
    if (length.isa( OTLong ))
    {
		BLong& lng = (BLong &) length;

        len = (int)lng.value();
    }
    else if (length.isa( OTDouble ))
    {
		Double& dbl = (Double &) length;

        len = (int)dbl.value();
    }
    else
    {
        return BObjectRef( copy() );
    }

    if (target->isa( OTString ))
    {
        String* target_str = (String*) target;
        value_.replace( index-1, len, target_str->value_ );
    }
    else
    {
        return BObjectRef( UninitObject::create() );
    }

    return BObjectRef( this );
}


BObjectRef String::OperMultiSubscript( stack<BObjectRef>& indices )
{
    BObjectRef start_ref = indices.top();
    indices.pop();
    BObjectRef length_ref = indices.top();
    indices.pop();

    BObject& length_obj = *length_ref;
    BObject& start_obj = *start_ref;

    BObjectImp& length = length_obj.impref();
    BObjectImp& start = start_obj.impref();

    // first deal with the start position.
    unsigned index;
    if (start.isa( OTLong ))
    {
        BLong& lng = (BLong&) start;
        index = (unsigned) lng.value();
        if (index == 0 || index > value_.size())
            return BObjectRef(new BError( "Subscript out of range" ));

    }
    else if (start.isa( OTString ))
    {
		String& rtstr = (String&) start;
        string::size_type pos = value_.find( rtstr.value_ );
        if (pos != string::npos)
            index = pos+1;
        else
            return BObjectRef(new UninitObject);
    }
    else
    {
        return BObjectRef(copy());
    }

    // now deal with the length.
    int len;
    if (length.isa( OTLong ))
    {
		BLong& lng = (BLong &) length;

        len = (int)lng.value();
    }
    else if (length.isa( OTDouble ))
    {
		Double& dbl = (Double &) length;

        len = (int)dbl.value();
    }
    else
    {
        return BObjectRef( copy() );
    }

    String* str = new String( value_, index-1, len );
    return BObjectRef( str );
}

BObjectRef String::OperSubscript( const BObject& rightobj )
{
    const BObjectImp& right = rightobj.impref();
    if (right.isa( OTLong ))
    {
		BLong& lng = (BLong& ) right;

		unsigned index = (unsigned)lng.value();
		
        if (index == 0 || index > value_.size())
            return BObjectRef(new BError( "Subscript out of range" ));

		return BObjectRef( new BObject( new String( value_.c_str()+index-1, 1 ) ) );
    }
    else if (right.isa( OTDouble ))
	{
		Double& dbl = (Double& ) right;

		unsigned index = (unsigned)dbl.value();
		
        if (index == 0 || index > value_.size())
            return BObjectRef(new BError( "Subscript out of range" ));

		return BObjectRef( new BObject( new String( value_.c_str()+index-1, 1 ) ) );
	}
	else if (right.isa( OTString ))
	{
		String& rtstr = (String&) right;
        string::size_type pos = value_.find( rtstr.value_ );
        if (pos != string::npos)
            return BObjectRef( new BObject( new String( value_, pos, 1 ) ) );
        else
            return BObjectRef(new UninitObject);
    }
    else
    {
        return BObjectRef(new UninitObject);
    }
}

