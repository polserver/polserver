
ReadConfigFile( filename ); // not including ".cfg"

// File scope functions
FindConfigElem( config_file, key ); // key may be an integer or a string.
GetConfigMaxIntKey( config_file );
GetConfigStringKeys( config_file ); // return an array of all the keys as strings
GetConfigIntKeys( config_file ); // return an array of all the int keys

// Element scope functions
ListConfigElemProps( element ); // : array of the element's property-names (unique strings)
GetConfigInt( element, property_name ); // : integer;
GetConfigReal( element, property_name ); // : real;
GetConfigString( element, property_name ); // : string;

// For elements with multiple occurrences of a given property,
// return an array containing each occurrence.
GetConfigStringArray( element, property_name ); // : array of strings;
GetConfigIntArray( element, property_name ); // array of ints
GetConfigStringDictionary( element, property_name );

// Appends an element to the config file specified.
// Properties must be an array of structures, where each element
// is a structure containing a name member first, and a value member
// second.
AppendConfigFileElem( filename, elemtype, elemkey, properties );

// Unloads a config file from memory.  If other scripts reference the
// config file specified, it will remain in memory until they no longer
// reference it.  Scripts which call ReadConfigFile() after this, however,
// will re-read the config file and use the newer version.
UnloadConfigFile( filename );

//
// Load a TUS .SCP file as a config file
//
// Filename must be alphanumeric only.  The file must exist
// in pol/import/tus.  So LoadTusScpFile( "grayacct" ) will
// try to load "/pol/import/tus/grayacct.scp"
//
// Header properties, if any, in cfgfile[0]
// Other elements numbered sequentially from 1
// The part in brackets ("[WORLDITEM 04000]") will appear in two ways:
//   1) as a "_key" property
//   2) as a property itself (name=WORLDITEM, value=0x4000)
//
//
// So,if you had the following as the first element
//   [WORLDITEM 04000]
//   SERIAL=080
//   P=1,2,3
// And 'var tusfile' was the result of LoadTusScpFile, then
// tusfile[1]._key      is "WORLDITEM 04000"
// tusfile[1].worlditem is 0x4000
// tusfile[1].serial    is 0x80
// tusfile[1].p         is "1,2,3"
//
LoadTusScpFile( filename );

// deprecated by GetConfigString:
GetElemProperty( element, property_name ); // : string;
