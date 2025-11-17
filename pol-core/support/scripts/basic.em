// format-off
const TRIM_LEFT  := 0x1; // Trim whitespace from Left of string.
const TRIM_RIGHT := 0x2; // Trim whitespace from Right of string.
const TRIM_BOTH  := 0x3; // Trim whitespace from Left and Right of string.

// TypeOfInt() constants
const OT_UNKNOWN          :=  0;
const OT_UNINIT           :=  1;
const OT_STRING           :=  2;
const OT_LONG             :=  3;
const OT_DOUBLE           :=  4;
const OT_ARRAY            :=  5;
const OT_APPLICPTR        :=  6;
const OT_APPLICOBJ        :=  7;
const OT_ERROR            :=  8;
const OT_DICTIONARY       :=  9;
const OT_STRUCT           := 10;
const OT_PACKET           := 11;
const OT_BINARYFILE       := 12;
const OT_XMLFILE          := 13;
const OT_XMLNODE          := 14;
const OT_XMLATTRIBUTES    := 15;
const OT_POLCOREREF       := 16;
const OT_ACCOUNTREF       := 17;
const OT_CONFIGFILEREF    := 18;
const OT_CONFIGELEMREF    := 19;
const OT_DATAFILEREF      := 20;
const OT_DATAELEMREF      := 21;
const OT_GUILDREF         := 22;
const OT_PARTYREF         := 23;
const OT_BOUNDINGBOX      := 24;
const OT_DEBUGCONTEXT     := 25;
const OT_SCRIPTEXREF      := 26;
const OT_PACKAGE          := 27;
const OT_MENUREF          := 28;
const OT_MOBILEREF        := 29;
const OT_OFFLINEMOBILEREF := 30;
const OT_ITEMREF          := 31;
const OT_BOATREF          := 32;
const OT_MULTIREF         := 33;
const OT_CLIENTREF        := 34;
const OT_SQLCONN          := 35; // Only applicable if SQL is active and compiled.
const OT_SQLRESULT        := 36;
const OT_SQLROW           := 37;
const OT_BOOLEAN          := 38;
const OT_FUNCOBJECT       := 39;
const OT_EXPORTEDSCRIPT   := 40;
const OT_STORAGEAREA      := 41;
const OT_CLASSINSTANCEREF := 42;
const OT_REGEXP           := 43;
// format-on

// returns the one-based index of Search within Str after position Start
Find( Str, Search, Start );

// len(): For strings, returns string length
//        For arrays, returns number of elements.
Len( Str_or_array );

// upper(): Convert whole string to upper-case
// lower(): Convert whole string to lower-case
Upper( Str );
Lower( Str );

// Various Conversion Functions
//
CInt( expr ); // Convert to integer
CDbl( expr ); // Convert to double
CStr( expr ); // Convert to string
CAsc( str ); // Convert first character to Ascii value (0-255)
CChr( number ); // Convert Ascii value to character (type: string)
CAscZ( str, nullterm := 0 ); // Convert string to array of Ascii values (0-255)
CChrZ( intarray, break_at_first_null := 0 ); // Convert array of Ascii values to a string

Bin( number ); // Convert 25 to 11001
Hex( number ); // Convert to 0x[hex val] - 0x0 to 0xFFFFFFFF

// Compare(): Compares 2 strings and returns based results.
// 1 = Match
// 0 = Not a match
Compare( str1, str2, pos1_start := 0, pos1_end := 0, pos2_start := 0, pos2_end := 0 );

// SplitWords: returns an array of words contained in str, based
// on optional delimiter. Default delim is blank space.
// on optional max_split. Default -1 for unlimited.
SplitWords( str, delimiter := " ", max_split := -1 );
// SubStr(): Returns substring beginning at start until length.
SubStr( str, start, length );
// Trim(): Returns string with whitespace cleared based on Type and Character to trim.
// set: Character set to trim
// type: 1 = Trim Leading, 2 = Trim Trailing, 3 = Trim Both.
Trim( str, type := TRIM_BOTH, to_trim := " " );
// StrReplace(): Replace instance of to_replace with replace_with in str.
StrReplace( str, to_replace, replace_with );
// SubStrReplace(): Replace portion of string using replace_with, beginning at start for length.
SubStrReplace( str, replace_with, start, length := 0 );

// Pack and Unpack: convert an object to a storable string form
// Pack( 5 ) returns "i5"
// Unpack( "i5" ) returns 5
// strings, integers, reals, and arrays and dictionaries of these can be packed.
Pack( expr );
Unpack( str );

TypeOf( expr ); // returns "Integer", "Real" etc
SizeOf( expr ); // returns estimate of memory used
TypeOfInt( expr ); // returns Integer representation of TypeOf string
PackJSON( expr, pretty := 0 );
UnpackJSON( jsonStr );
Boolean( number ); // returns boolean instance
EncodeBase64( str );
DecodeBase64( str );
RegExp( pattern, flags := "" );
