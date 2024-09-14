//
// OpenDataFile flags:
//
// can specify only one of the following:
const DF_KEYTYPE_STRING := 0x00; // default
const DF_KEYTYPE_INTEGER := 0x01;

//
// Functions
//
ListDataFiles();
CreateDataFile( filespec, flags := DF_KEYTYPE_STRING );
OpenDataFile( filespec ); // fails if does not exist
// DataFileExists( filespec );
// DeleteDataFile( filespec );
// RenameDataFile( oldfilespec, newfilespec );
UnloadDataFile( filespec );

// Object: DataFile datafile;
// datafile methods:
//  datafile.FindElement( key : string|integer ) : elemref
//  datafile.CreateElement( key : string|integer ) : elemref
//  datafile.DeleteElement( key : string|integer );
//  datafile.Keys() : array of (string|integer)

// Object: DataFileElement elem;
// DataFileElement methods:
//  elem.GetProp( propname : string );
//  elem.SetProp( propname : string, propvalue : packable );
//  elem.EraseProp( propname : string );
//  elem.PropNames();
