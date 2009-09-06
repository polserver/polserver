/*
History
=======
2005/11/24 Shinigami: added itoa for Linux because it is not ANSI C/C++
2005/11/29 Shinigami: mf_SplitWords will now accept each type of to-split-value as same as in the past
2006/10/07 Shinigami: FreeBSD fix - changed __linux__ to __unix__
2006/12/29 Shinigami: mf_SplitWords will not hang server on queue of delimiter

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include <stdio.h>
#include <string.h>


#include "../../clib/clib.h"
#include "../../clib/rawtypes.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include "basicmod.h"

BObjectImp* BasicExecutorModule::len()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if (imp->isa( BObjectImp::OTArray ))
	{
		ObjArray* arr = static_cast<ObjArray*>(imp);
		return new BLong( arr->ref_arr.size() );
	}
	else if (imp->isa(BObjectImp::OTString))
	{
		return new BLong( imp->getStringRep().length() );
	}
	else if (imp->isa( BObjectImp::OTError ))
	{
		BError* err = static_cast<BError*>(imp);
		return new BLong( err->mapcount() );
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* BasicExecutorModule::find()
{
	exec.makeString(0);
	String* str = static_cast<String*>(exec.getParamImp(0));
	const char *s = exec.paramAsString(1);
	int d = static_cast<int>(exec.paramAsLong(2));

	int posn = str->find(d ? (d - 1) : 0, s) + 1;

	return new BLong (posn);
}

BObjectImp* BasicExecutorModule::mf_substr()
{
	exec.makeString(0);
	String* str = static_cast<String*>(exec.getParamImp(0));
	int start = static_cast<int>(exec.paramAsLong(1));
	int length = static_cast<int>(exec.paramAsLong(2));
	
	return str->StrStr(start, length);
}

BObjectImp* BasicExecutorModule::mf_Trim()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if ( !(imp->isa(BObjectImp::OTString)) )
	{
		return new BError("Param 1 must be a string.");
	}
	String *string = static_cast<String*>(imp);
	int type = static_cast<int>(exec.paramAsLong(1));
	const char* cset = exec.paramAsString(2);
	if ( type > 3 ) 
		type = 3;
	if ( type < 1 )
		type = 1;

	return string->ETrim(cset, type);
}

/*
eScript Function Useage

StrReplace(str, to_replace, replace_with);
*/
BObjectImp* BasicExecutorModule::mf_StrReplace()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	String *string = new String(imp->getStringRep().c_str());
	String *to_replace = static_cast<String*>(exec.getParamImp(1));
	String *replace_with = static_cast<String*>(exec.getParamImp(2));

	if (string->length() < 1)
		return new BError("Cannot use empty string for string haystack.");
	if (to_replace->length() < 1)
		return new BError("Cannot use empty string for string needle.");

    string->EStrReplace(to_replace, replace_with);

	return string;
}

// SubStrReplace(str, replace_with, start, length:=0); 
BObjectImp* BasicExecutorModule::mf_SubStrReplace()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	String *string = new String(imp->getStringRep().c_str());
	String *replace_with = static_cast<String*>(exec.getParamImp(1));
	unsigned int index = static_cast<int>(exec.paramAsLong(2));
	unsigned int len = static_cast<int>(exec.paramAsLong(3));

	if ( index < 0 )
		return new BError("Index must not be negative");
	if ( (index-1) > string->length() )
		return new BError("Index out of range");

	// We set it to 1 because of doing -1 later to stay with eScript handling.
	if ( !index )
		index = 1;

	if ( len > (string->length()-index) )
		return new BError("Length out of range");
	if ( len < 0 )
		return new BError("Length must not be negative");

	if ( !len )
		len = replace_with->length() - index;

    string->ESubStrReplace(replace_with, index, len);

	return string;
}

// OMG I HATED THIS REQUEST. Ugly code, but necessary for all the checks
// just in case someone's code is bugged :(
BObjectImp* BasicExecutorModule::mf_Compare()
{
	string str1 = exec.paramAsString(0);
	string str2 = exec.paramAsString(1);
	unsigned int pos1_index = static_cast<int>(exec.paramAsLong(2));
	unsigned int pos1_len = static_cast<int>(exec.paramAsLong(3));
	unsigned int pos2_index = static_cast<int>(exec.paramAsLong(4));
	unsigned int pos2_len = static_cast<int>(exec.paramAsLong(5));

	if ( pos1_index != 0 )
	{
		if ( pos1_index < 0 )
			return new BError("Index must not be negative for param 1");
		if ( (pos1_index-1) > str1.length() )
			return new BError("Index out of range for param 1");
	}
	if ( pos2_index != 0 )
	{
		if ( pos2_index < 0 )
			return new BError("Index must not be negative for param 2");
		if ( (pos2_index-1) > str2.length() )
			return new BError("Index out of range for param 2");
	}

	if ( pos1_len > (str1.length()-pos1_index) )
		return new BError("Length out of range for param 1");
	if ( pos1_len < 0 )
		return new BError("Length must not be negative for param 1");
	if ( pos2_len > (str2.length()-pos2_index) )
		return new BError("Length out of range for param 2");
	if ( pos2_len < 0 )
		return new BError("Length must not be negative for param 2");

	if ( pos1_index == 0 )
	{
		unsigned int result = str1.compare(str2);
		if ( result != 0 )
			return new BLong(0);
		else
			return new BLong(1);
	}
	else if ( pos1_index > 0 && pos2_index == 0 )
	{
		unsigned int result = str1.compare(pos1_index-1, pos1_len, str2);
		if ( result != 0 )
			return new BLong(0);
		else
			return new BLong(1);
	}
	else
	{
		unsigned int result = str1.compare(pos1_index-1, pos1_len, str2, pos2_index-1, pos2_len);
		if ( result != 0 )
			return new BLong(0);
		else
			return new BLong(1);
	}
}

BObjectImp* BasicExecutorModule::lower()
{
	String *string = new String( exec.paramAsString(0) );
	string->toLower();
	return string;
}

BObjectImp* BasicExecutorModule::upper()
{
	String *string = new String( exec.paramAsString(0) );
	string->toUpper();
	return string;
}

BObjectImp* BasicExecutorModule::mf_CInt()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if (imp->isa( BObjectImp::OTLong ))
	{
		return imp->copy();
	}
	else if (imp->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(imp);
		return new BLong( strtoul( str->data(), NULL, 0 ) );
	}
	else if (imp->isa( BObjectImp::OTDouble ))
	{
		Double* dbl = static_cast<Double*>(imp);
		return new BLong( static_cast<long>(dbl->value()) );
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* BasicExecutorModule::mf_CDbl()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if (imp->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(imp);
		return new Double( lng->value() );
	}
	else if (imp->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(imp);
		return new Double( strtod( str->data(), NULL ) );
	}
	else if (imp->isa( BObjectImp::OTDouble ))
	{
		return imp->copy();
	}
	else
	{
		return new Double(0);
	}
}

BObjectImp* BasicExecutorModule::mf_CStr()
{
	BObjectImp* imp = exec.getParamImp(0);
	return new String( imp->getStringRep() );
}

BObjectImp* BasicExecutorModule::mf_CAsc()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	if (imp->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(imp);
		return new BLong( static_cast<unsigned char>(str->data()[0]) );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* BasicExecutorModule::mf_CAscZ()
{
	BObjectImp* imp = exec.getParamImp( 0 );
	string tmp = imp->getStringRep();
	ObjArray* arr = new ObjArray;
	for( unsigned i = 0; i < tmp.size(); ++i )
	{
		arr->addElement( new BLong( static_cast<unsigned char>(tmp[i]) ) );
	}
	return arr;
}

BObjectImp* BasicExecutorModule::mf_CChr()
{
	long val;
	if (getParam( 0, val ))
	{
		char s[2];
		s[0] = static_cast<char>(val);
		s[1] = '\0';
		return new String( s );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* BasicExecutorModule::mf_CChrZ()
{
	string res;
	ObjArray* arr = static_cast<ObjArray*>(exec.getParamImp( 0, BObjectImp::OTArray ));
	if (!arr)
		return new BError( "Invalid parameter type" );
	for( ObjArray::const_iterator itr = arr->ref_arr.begin(); itr != arr->ref_arr.end(); itr++ )
	{
		BObjectImp* imp = (*itr).get()->impptr();
		if (imp)
		{
			if (imp->isa( BObjectImp::OTLong ))
			{
				BLong* blong = static_cast<BLong*>(imp);
				char s[2];
				s[0] = static_cast<char>(blong->value());
				s[1] = '\0';
				res += s;
			}
		}

	}
	return new String(res);

}

BObjectImp* BasicExecutorModule::mf_Hex()
{
	BObjectImp* imp = exec.getParamImp(0);
	if (imp->isa( BObjectImp::OTLong ))
	{
		BLong* plong = static_cast<BLong*>(imp);
		char s[20];
		sprintf( s, "0x%lX", plong->value() );
		return new String( s );
	}
	else if (imp->isa( BObjectImp::OTDouble ))
	{
		Double* pdbl = static_cast<Double*>(imp);
		char s[20];
		sprintf( s, "0x%lX", static_cast<unsigned long>(pdbl->value()) );
		return new String( s );
	}
	else if (imp->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(imp);
		char s[20];
		sprintf( s, "0x%lX", strtoul( str->data(), NULL, 0 ) );
		return new String( s );
	}
	else
	{
		return new BError( "Hex() expects an Integer, Real, or String" );
	}
}

#ifdef __unix__
char* itoa( long value, char* result, long base ) {
	// check that the base is valid
	if (base < 2 || base > 16) { *result = 0; return result; }
	
	char* out = result;
	long quotient = value;
	
	do {
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		++out;
		quotient /= base;
	} while ( quotient );
	
	// Only apply negative sign for base 10
	if ( value < 0 && base == 10) *out++ = '-';
	
	std::reverse( result, out );
	*out = 0;
	return result;
}
#endif

BObjectImp* BasicExecutorModule::mf_Bin()
{
	BObjectImp* imp = exec.getParamImp(0);
	if (imp->isa( BObjectImp::OTLong ))
	{
		BLong* plong = static_cast<BLong*>(imp);
		long number = plong->value();
		char buffer [sizeof(long)*8+1];
		return new String(itoa(number, buffer, 2));
	}
	else
	{
		return new BError( "Bin() expects an Integer" );
	}
}

int bpx;
void bphelp()
{
	++bpx;
}

BObjectImp* BasicExecutorModule::mf_SplitWords()
{
	BObjectImp* bimp_split = exec.getParamImp(0);
	string source = bimp_split->getStringRep();

	const String* bimp_delimiter;
	if ( !exec.getStringParam(1, bimp_delimiter) )
	{
		return new BError("Invalid parameter type.");
	}
	string delimiter = bimp_delimiter->getStringRep();

	ObjArray* objarr = new ObjArray;

	// Support for how it previously worked.
	// Kept to support spaces and tabs as the same.
	if ( delimiter == " " )
	{
		ISTRINGSTREAM is(source);
		string tmp;
		while ( is >> tmp )
		{
			objarr->addElement(new String(tmp));
			tmp = "";
		}
		return objarr;
	}

	// New delimiter support.
	string new_string = source;
	string tmp_string, add_string;
	string::size_type found;
	do
	{
		found = new_string.find(delimiter, 0);
		if ( found == string::npos )
			break;
		
		add_string = new_string.substr(0, found);
		
		// Shinigami: will not hang server on queue of delimiter
		// if ( add_string.empty() )
		//	continue;
		
		objarr->addElement(new String(add_string));
		tmp_string = new_string.substr(found+delimiter.length(), new_string.length());
		new_string = tmp_string;
	}
	while ( found != string::npos/*-1*/ );

	// Catch leftovers here.
	if ( !new_string.empty() )
		objarr->addElement(new String(new_string));

	return objarr;
}

BObjectImp* BasicExecutorModule::mf_Pack()
{
	BObjectImp* imp = exec.getParamImp(0);
	return new String( imp->pack() );
}

BObjectImp* BasicExecutorModule::mf_Unpack()
{
	const String* str;

	if (exec.getStringParam( 0, str ))
	{
		return BObjectImp::unpack( str->data() );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}
BObjectImp* BasicExecutorModule::mf_TypeOf()
{
	BObjectImp* imp = exec.getParamImp(0);
	return new String( imp->typeOf() );
}
BObjectImp* BasicExecutorModule::mf_SizeOf()
{
	BObjectImp* imp = exec.getParamImp(0);
	return new BLong( imp->sizeEstimate() );
}

BasicFunctionDef BasicExecutorModule::function_table[] =
{
	{ "find",			&BasicExecutorModule::find },
	{ "len",			&BasicExecutorModule::len },
	{ "upper",			&BasicExecutorModule::upper },
	{ "lower",			&BasicExecutorModule::lower },
	{ "Substr",			&BasicExecutorModule::mf_substr },
	{ "Trim",			&BasicExecutorModule::mf_Trim },
	{ "StrReplace",		&BasicExecutorModule::mf_StrReplace },
	{ "SubStrReplace",	&BasicExecutorModule::mf_SubStrReplace },
	{ "Compare",		&BasicExecutorModule::mf_Compare },
	{ "CInt",			&BasicExecutorModule::mf_CInt },
	{ "CStr",			&BasicExecutorModule::mf_CStr },
	{ "CDbl",			&BasicExecutorModule::mf_CDbl },
	{ "CAsc",			&BasicExecutorModule::mf_CAsc },
	{ "CChr",			&BasicExecutorModule::mf_CChr },
	{ "CAscZ",			&BasicExecutorModule::mf_CAscZ },
	{ "CChrZ",			&BasicExecutorModule::mf_CChrZ },
	{ "Bin",			&BasicExecutorModule::mf_Bin },
	{ "Hex",			&BasicExecutorModule::mf_Hex },
	{ "SplitWords",		&BasicExecutorModule::mf_SplitWords },
	{ "Pack",			&BasicExecutorModule::mf_Pack },
	{ "Unpack",			&BasicExecutorModule::mf_Unpack },
	{ "TypeOf",			&BasicExecutorModule::mf_TypeOf },
	{ "SizeOf",			&BasicExecutorModule::mf_SizeOf }
};

int BasicExecutorModule::functionIndex( const char *name )
{
	for( unsigned idx = 0; idx < arsize(function_table); idx++ )
	{
		if (stricmp( name, function_table[idx].funcname ) == 0)
			return idx;
	}
	return -1;
}

BObjectImp* BasicExecutorModule::execFunc( unsigned funcidx )
{
	return callMemberFunction(*this, function_table[funcidx].fptr)();
};
