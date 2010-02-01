const TRIM_LEFT  := 0x1; // Trim whitespace from Left of string.
const TRIM_RIGHT := 0x2; // Trim whitespace from Right of string.
const TRIM_BOTH  := 0x3; // Trim whitespace from Left and Right of string.

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
CInt( expr );           // Convert to integer
CDbl( expr );           // Convert to double
CStr( expr );           // Convert to string
CAsc( str );            // Convert first character to Ascii value (0-255)
CChr( number );         // Convert Ascii value to character (type: string)
CAscZ( str );           // Convert string to array of Ascii values (0-255)
CChrZ( intarray );      // Convert array of Ascii values to a string

Bin( number) ;			// Convert 25 to 11001
Hex( number );          // Convert to 0x[hex val] - 0x0 to 0xFFFFFFFF

	// Compare(): Compares 2 strings and returns based results.
	// 1 = Match
	// 0 = Not a match
Compare(str1, str2, pos1_start:=0, pos1_end:=0, pos2_start:=0, pos2_end:=0);
	
	// SplitWords: returns an array of words contained in str, based
	// on optional delimiter. Default delim is blank space.
SplitWords( str, delimiter := " " );
	// SubStr(): Returns substring beginning at start until length.
SubStr(str, start, length);
	// Trim(): Returns string with whitespace cleared based on Type and Character to trim.
	// set: Character set to trim
	// type: 1 = Trim Leading, 2 = Trim Trailing, 3 = Trim Both.
Trim(str, type:=TRIM_BOTH, to_trim:=" ");
	// StrReplace(): Replace instance of to_replace with replace_with in str.
StrReplace(str, to_replace, replace_with);
	// SubStrReplace(): Replace portion of string using replace_with, beginning at start for length.
SubStrReplace(str, replace_with, start, length:=0);

    // Pack and Unpack: convert an object to a storable string form
    // Pack( 5 ) returns "i5"
    // Unpack( "i5" ) returns 5
    // strings, integers, reals, and arrays and dictionaries of these can be packed.
Pack( expr );
Unpack( str );

TypeOf( expr ); // returns "Integer", "Real" etc
SizeOf( expr ); // returns estimate of memory used
