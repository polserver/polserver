////////////////////////////////////////////////////////////////
//
//  CONSTANTS
//
////////////////////////////////////////////////////////////////

// format-off
// LogToFile constants
const LOG_DATETIME := 1; // log Core-Style DateTimeStr in front of log entry

// BinaryFile constants
const OPENMODE_IN    := 0x01; // (input) Allow input operations on the stream
const OPENMODE_OUT   := 0x02; // (output) Allow output operations on the stream
const OPENMODE_ATE   := 0x04; // (at end) Set the stream's position indicator to the end of the stream on opening
const OPENMODE_APP   := 0x08; // (append) Set the stream's position indicator to the end of the stream before each output operation
const OPENMODE_TRUNC := 0x10; // (truncate) Any current content is discarded, assuming a length of zero on opening

const SEEKDIR_BEG := 0x00; // beginning of sequence
const SEEKDIR_CUR := 0x01; // current position within sequence
const SEEKDIR_END := 0x02; // end of sequence

const LITTLE_ENDIAN := 0x00;
const BIG_ENDIAN    := 0x01;

// ListDirectory constants
const LISTDIR_LIST_DIRS      := 0x1;
const LISTDIR_DONT_LIST_DIRS := 0x0;
const LISTDIR_NO_FILES       := "";
const LISTDIR_ALL_FILES      := "*";
// format-on

////////////////////////////////////////////////////////////////
//
//  FUNCTIONS
//
////////////////////////////////////////////////////////////////

FileExists( filename );
ReadFile( filename );
WriteFile( filename, textcontents );
AppendToFile( filename, textlines );
LogToFile( filename, line, flags := 0 );

OpenBinaryFile( filename, mode := OPENMODE_IN, endian := LITTLE_ENDIAN );

CreateDirectory( dirname );
ListDirectory( dirname, extension := LISTDIR_ALL_FILES, listdirs := LISTDIR_DONT_LIST_DIRS );

OpenXMLFile( filename );
CreateXMLFile();
