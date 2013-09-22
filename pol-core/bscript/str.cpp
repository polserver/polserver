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
#include <boost/foreach.hpp>

#ifdef __GNUG__
#	include <streambuf>
#endif

#include "berror.h"
#include "bobject.h"
#include "impstr.h"
#include "objmethods.h"
#include "executor.h"
#include "../clib/stlutil.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4244 )
#endif

String::String(BObjectImp& objimp) : 
	BObjectImp( OTString ),
	value_(objimp.getStringRep())
{
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

size_t String::sizeEstimate() const
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
        return static_cast<int>(pos);
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
	int strlen = static_cast<int>(this->length());
	for ( int i=0; i < strlen; i++ )
	{
		unsigned char tmp = value_[i];
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
    return objimp.selfPlusObj(*this);
}
BObjectImp* String::selfPlusObj(const BObjectImp& objimp) const
{
	 return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj(const BLong& objimp) const
{
	 return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj(const Double& objimp) const
{
	 return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj(const String& objimp) const
{
	 return new String( value_ + objimp.getStringRep() );
}
BObjectImp* String::selfPlusObj(const ObjArray& objimp) const
{
	 return new String( value_ + objimp.getStringRep() );
}
void String::selfPlusObjImp(BObjectImp& objimp , BObject& obj)
{
	objimp.selfPlusObj(*this,obj);
}
void String::selfPlusObj(BObjectImp& objimp, BObject& obj)
{
	value_ += objimp.getStringRep();
}
void String::selfPlusObj(BLong& objimp, BObject& obj)
{
	value_ += objimp.getStringRep();
}
void String::selfPlusObj(Double& objimp, BObject& obj)
{
	value_ += objimp.getStringRep();
}
void String::selfPlusObj(String& objimp, BObject& obj)
{
	value_ += objimp.getStringRep();
}
void String::selfPlusObj(ObjArray& objimp, BObject& obj)
{
	value_ += objimp.getStringRep();
}


void String::remove(const char *rm)
{
    size_t len = strlen(rm);
    string::size_type pos = value_.find( rm );
    if (pos != string::npos)
        value_.erase( pos, len );
}

BObjectImp* String::selfMinusObjImp(const BObjectImp& objimp) const
{
    return objimp.selfMinusObj(*this);
}
BObjectImp* String::selfMinusObj(const BObjectImp& objimp) const
{
	String *tmp = (String *) copy();
	tmp->remove(objimp.getStringRep().data());
	return tmp;
}
BObjectImp* String::selfMinusObj(const BLong& objimp) const
{
	String *tmp = (String *) copy();
	tmp->remove(objimp.getStringRep().data());
	return tmp;
}
BObjectImp* String::selfMinusObj(const Double& objimp) const
{
	String *tmp = (String *) copy();
	tmp->remove(objimp.getStringRep().data());
	return tmp;
}
BObjectImp* String::selfMinusObj(const String& objimp) const
{
	String *tmp = (String *) copy();
    tmp->remove(objimp.value_.data());
	return tmp;
}
BObjectImp* String::selfMinusObj(const ObjArray& objimp) const
{
	String *tmp = (String *) copy();
	tmp->remove(objimp.getStringRep().data());
	return tmp;
}
void String::selfMinusObjImp(BObjectImp& objimp , BObject& obj)
{
	objimp.selfMinusObj(*this,obj);
}
void String::selfMinusObj(BObjectImp& objimp, BObject& obj)
{
	remove(objimp.getStringRep().data());
}
void String::selfMinusObj(BLong& objimp, BObject& obj)
{
	remove(objimp.getStringRep().data());
}
void String::selfMinusObj(Double& objimp, BObject& obj)
{
	remove(objimp.getStringRep().data());
}
void String::selfMinusObj(String& objimp, BObject& obj)
{
    remove(objimp.value_.data());
}
void String::selfMinusObj(ObjArray& objimp, BObject& obj)
{
	remove(objimp.getStringRep().data());
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
	BOOST_FOREACH(char &c, value_)
	{
		c = toupper(c);
	}
}

void String::toLower( void )
{
	BOOST_FOREACH(char &c, value_)
	{
		c = tolower(c);
	}
}

BObjectImp* String::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
	(void) copy;
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
            index = static_cast<unsigned int>(pos+1);
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
            index = static_cast<unsigned int>(pos+1);
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

    auto str = new String( value_, index-1, len );
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

// -- format related stuff --

bool s_parse_int(int &i, string const &s)
{	
	if(s.empty())
		return false;

	char* end;
	i = strtol(s.c_str(), &end, 10);

	if (!*end) {
		return true;
	}
	else {		
		return false;
	}
}

// remove leading/trailing spaces
void s_trim(string &s)
{
	std::stringstream trimmer;
	trimmer << s;
	s.clear();
	trimmer >> s;
}

void int_to_binstr(int& value, std::stringstream &s)
{
	int i;
	for(i = 31; i > 0; i--) {
		if(value & (1 << i))
			break;
	}
	for(; i >= 0; i--) {
		if(value & (1 << i))
			s << "1";
		else
			s << "0";
	}
}

bool try_to_format( std::stringstream &to_stream, BObjectImp *what, string& frmt )
{
	if(frmt.empty()) {
		to_stream << what->getStringRep();
		return false;
	}

	if(frmt.find('b')!=string::npos) {
		if(!what->isa( BObjectImp::OTLong )) {
			to_stream << "<int required>";
			return false;
		}
		BLong* plong = static_cast<BLong*>(what);
		int n = plong->value();
		if(frmt.find('#')!=string::npos)
			to_stream << ((n<0)?"-":"") << "0b";
		int_to_binstr(n, to_stream);
	} else if(frmt.find('x')!=string::npos) {
		if(!what->isa( BObjectImp::OTLong )) {
			to_stream << "<int required>";
			return false;
		}
		BLong* plong = static_cast<BLong*>(what);
		int n = plong->value();
		if(frmt.find('#')!=string::npos)
			to_stream << "0x";
		to_stream << std::hex << n;
	} else if(frmt.find('o')!=string::npos) {
		if(!what->isa( BObjectImp::OTLong )) {
			to_stream << "<int required>";
			return false;
		}
		BLong* plong = static_cast<BLong*>(what);
		int n = plong->value();
		if(frmt.find('#')!=string::npos)
			to_stream << "0o";
		to_stream << std::oct << n;
	} else if(frmt.find('d')!=string::npos) {
		int n;
		if(what->isa( BObjectImp::OTLong )) {
			BLong* plong = static_cast<BLong*>(what);
			n = plong->value();
		} else if(what->isa( BObjectImp::OTDouble )) {
			Double* pdbl = static_cast<Double*>(what);
			n = (int) pdbl->value();
		} else {
			to_stream << "<int or double required>";
			return false;
		}		
		to_stream << std::dec << n;
	} else {
		to_stream << "<unknown format: " << frmt << ">";
		return false;
	}		
	return true;
}

// --


BObjectImp* String::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}
BObjectImp* String::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	switch (id)
	{
	case MTH_LENGTH:
		if (ex.numParams() == 0)
			return new BLong( static_cast<int>(value_.length()) );
		else
			return new BError( "string.length() doesn't take parameters." );
		break;
	case MTH_FIND:
		{
			if (ex.numParams() > 2)
				return new BError("string.find(Search, [Start]) takes only two parameters");
			if (ex.numParams() < 1)
				return new BError("string.find(Search, [Start]) takes at least one parameter");
			const char *s = ex.paramAsString(0);
			int d = 0;
			if (ex.numParams() == 2)
				d = ex.paramAsLong(1);
			int posn = find(d ? (d - 1) : 0, s) + 1;
			return new BLong (posn);
		}
	case MTH_UPPER:
		{
			if (ex.numParams() == 0)
			{
				toUpper();
				return this;
			}
			else
				return new BError( "string.upper() doesn't take parameters." );
		}

	case MTH_LOWER:
		{
			if (ex.numParams() == 0)
			{
				toLower();
				return this;
			}
			else
				return new BError( "string.lower() doesn't take parameters." );
		}
	case MTH_FORMAT:
		{
			if(ex.numParams() > 0) {

			string s = this->getStringRep(); // string itself
			std::stringstream result;

			size_t tag_start_pos; // the position of tag's start "{"
			size_t tag_stop_pos;  // the position of tag's end "}"
			size_t tag_dot_pos;
			int tag_param_idx;			

			size_t str_pos=0; // current string position		
			unsigned int next_param_idx = 0; // next index of .format() parameter

			while((tag_start_pos = s.find("{", str_pos)) != string::npos) {
				if((tag_stop_pos=s.find("}", tag_start_pos)) != string::npos) {

					result << s.substr(str_pos, tag_start_pos - str_pos);
					str_pos = tag_stop_pos + 1;
					
					string tag_body = s.substr(tag_start_pos+1, (tag_stop_pos - tag_start_pos)-1);					
					s_trim( tag_body ); // trim the tag of whitespaces

					string frmt;
					size_t formatter_pos = tag_body.find(':');

					if( formatter_pos!=string::npos) {
						frmt = tag_body.substr(formatter_pos + 1, string::npos); //
						tag_body = tag_body.substr(0, formatter_pos); // remove property from the tag
					}					

					string prop_name;									
					// parsing {1.this_part}
					tag_dot_pos = tag_body.find(".", 0);

					// '.' is found within the tag, there is a property name
					if(tag_dot_pos!=string::npos) {
						prop_name = tag_body.substr(tag_dot_pos + 1, string::npos); //
						tag_body = tag_body.substr(0, tag_dot_pos); // remove property from the tag

						// if s_tag_body is numeric then use it as an index
						if(s_parse_int(tag_param_idx, tag_body)) {
							tag_param_idx -= 1; // sinse POL indexes are 1-based
						} else {
							result << "<idx required before: '"<< prop_name <<"'>";
							continue;
						}						
					} else {						
						if(s_parse_int(tag_param_idx, tag_body)) {
							tag_param_idx -= 1; // sinse POL indexes are 1-based
						}
						else { // non-integer body has just next idx in line
							prop_name = tag_body;
							tag_param_idx = next_param_idx++;
						}
					}

					// -- end of property parsing

					//cout << "prop_name: '" << prop_name << "' tag_body: '" << tag_body << "'";

					// Checks that tag_param_idx is >0, otherwise it would fail when compared with ex.numParams()
					if( tag_param_idx < 0 || ex.numParams() <= static_cast<size_t>(tag_param_idx) ) {						
						result << "<idx out of range: #" << (tag_param_idx + 1) << ">";						
						continue;
					}					

					BObjectImp *imp = ex.getParamImp(tag_param_idx);

					if(prop_name.empty()==false) { // accesing object member
						BObjectRef obj_member = imp->get_member(prop_name.c_str());
						BObjectImp *member_imp = obj_member->impptr();
						try_to_format(result, member_imp, frmt);				
					} else {			
						try_to_format(result, imp, frmt);
					}
				} else {
					break;
				}
			}

			if( str_pos < s.length() ) {
				result << s.substr( str_pos, string::npos ); 
			}					

			return new String( result.str() );
			
		} else {
			return new BError( "string.format() requires a parameter." );
		}

	}
	case MTH_JOIN:
		{
			BObject* cont;
			if (ex.numParams() == 1 &&
				(cont = ex.getParamObj( 0 )) != NULL )
			{
				if (! (cont->isa( OTArray )))
					return new BError( "string.join expects an array" );
				ObjArray* container = static_cast<ObjArray*>(cont->impptr());
				// no empty check here on purpose
				OSTRINGSTREAM joined;
				bool first=true;
				BOOST_FOREACH(const BObjectRef &ref, container->ref_arr)
				{
					if ( ref.get() )
					{
						BObject *bo = ref.get();

						if ( bo == NULL )
							continue;
						if (!first)
							joined << value_;
						else
							first = false;
						joined << bo->impptr()->getStringRep();
					}
				}
				return new String(OSTRINGSTREAM_STR(joined));
			}
			else
				return new BError( "string.join(array) requires a parameter." );
		}
	default:
		return NULL;
	}
}
