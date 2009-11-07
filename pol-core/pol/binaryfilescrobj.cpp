/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/strutil.h"
#include "../clib/endian.h"

#include "binaryfilescrobj.h"

#include "../bscript/executor.h"
#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"



BBinaryfile::BBinaryfile(): BObjectImp( OTBinaryFile ),
_filename( "" ),
openmode( ios::in ),
bigendian( true )
{
}

BBinaryfile::BBinaryfile(string filename, unsigned short mode, bool _bigendian) : BObjectImp(OTBinaryFile),
_filename( filename ),
bigendian( _bigendian )
{
	//FIXME: ms::stl has different flag values then stlport :(
	openmode=static_cast<ios::openmode>(0x0);
	if (mode & 0x01)
		openmode |= ios::in;
	if (mode & 0x02)
		openmode |= ios::out;
	if (mode & 0x04)
		openmode |= ios::ate;
	if (mode & 0x08)
		openmode |= ios::app;
	if (mode & 0x10)
		openmode |= ios::trunc;

    if (!file.Open( filename, openmode ))
        return;
}

BBinaryfile::~BBinaryfile()
{
    file.Close();
}

BObjectRef BBinaryfile::get_member_id( const int id )//id test
{
    return BObjectRef(new BLong(0));
}
BObjectRef BBinaryfile::get_member( const char* membername )
{
    ObjMember* objmember = getKnownObjMember(membername);
    if( objmember != NULL )
        return this->get_member_id(objmember->id);
    else
        return BObjectRef(UninitObject::create());
}

BObjectImp* BBinaryfile::call_method( const char* methodname, Executor& ex )
{
    ObjMethod* objmethod = getKnownObjMethod(methodname);
    if( objmethod != NULL )
        return this->call_method_id(objmethod->id, ex);
    else
        return NULL;
}

BObjectImp* BBinaryfile::call_method_id( const int id, Executor& ex )
{
    switch(id)
    {
    case MTH_CLOSE:
        file.Close();
        return new BLong(1);
    case MTH_SIZE:
        return new BLong(static_cast<long>(file.FileSize(ex)));
    case MTH_SEEK:
        {
            if(ex.numParams() != 2)
                return new BError( "Seek requires 2 parameter." );
            long value, type;
            if((!ex.getParam( 0, value )) ||
                (!ex.getParam( 1, type )))
                return new BError("Invalid parameter");
            // FIXME: ms::stl has different flag values then stlport :(
            ios::seekdir seekdir;
            if (type & 0x01)
                seekdir = ios::cur;
            else if (type & 0x02)
                seekdir = ios::end;
            else 
                seekdir = ios::beg;
            if (!file.Seek(value,seekdir))
                return new BLong(0);
            return new BLong(1);
        }
    case MTH_TELL:
        return new BLong(static_cast<long>(file.Tell()));
    case MTH_PEEK:
        return new BLong(file.Peek());
    case MTH_FLUSH:
        file.Flush();
        return new BLong(1);

    case MTH_GETINT32:
        {
            u32 _u32;
            if (!file.Read(_u32))
                return new BError("Failed to read");
            if (bigendian)
                _u32=cfBEu32(_u32);
            return new BLong(_u32);
        }
    case MTH_GETSINT32:
        {
            s32 _s32;
            if (!file.Read(_s32))
                return new BError("Failed to read");
            if (bigendian)
                _s32=cfBEu32(_s32);
            return new BLong(_s32);
        }
    case MTH_GETINT16:
        {
            u16 _u16;
            if (!file.Read(_u16))
                return new BError("Failed to read");
            if (bigendian)
                _u16=cfBEu16(_u16);
            return new BLong(_u16);
        }
    case MTH_GETSINT16:
        {
            s16 _s16;
            if (!file.Read(_s16))
                return new BError("Failed to read");
            if (bigendian)
                _s16=cfBEu16(_s16);
            return new BLong(_s16);
        }
    case MTH_GETINT8:
        {
            u8 _u8;
            if (!file.Read(_u8))
                return new BError("Failed to read");
            return new BLong(_u8);
        }
    case MTH_GETSINT8:
        {
            s8 _s8;
            if (!file.Read(_s8))
                return new BError("Failed to read");
            return new BLong(_s8);
        }
    case MTH_GETSTRING:
        {
            if(ex.numParams() != 1)
                return new BError( "GetString requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            vector<unsigned char> _char;
            _char.resize(value);
            if (!file.Read(&_char[0],value))
                return new BError("Failed to read");
            int len = 0;
            const char* _str = reinterpret_cast<const char*>(&_char[0]);
            // Returns maximum of len characters or up to the first null-byte
            while (len < value && *(_str+len))
                len++;
            return new String( _str, len);
        }

    case MTH_SETINT32:
        {
            if(ex.numParams() != 1)
                return new BError( "SetInt32 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            u32 _u32=static_cast<u32>(value);
            if (bigendian)
                _u32=cfBEu32(_u32);
            if (!file.Write(_u32))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETSINT32:
        {
            if(ex.numParams() != 1)
                return new BError( "SetSInt32 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            s32 _s32=static_cast<s32>(value);
            if (bigendian)
                _s32=cfBEu32(_s32);
            if (!file.Write(_s32))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETINT16:
        {
            if(ex.numParams() != 1)
                return new BError( "SetInt16 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            u16 _u16=static_cast<u16>(value);
            if (bigendian)
                _u16=cfBEu16(_u16);
            if (!file.Write(_u16))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETSINT16:
        {
            if(ex.numParams() != 1)
                return new BError( "SetSInt16 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            s16 _s16=static_cast<s16>(value);
            if (bigendian)
                _s16=cfBEu16(_s16);
            if (!file.Write(_s16))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETINT8:
        {
            if(ex.numParams() != 1)
                return new BError( "SetInt8 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            u8 _u8=static_cast<u8>(value);
            if (!file.Write(_u8))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETSINT8:
        {
            if(ex.numParams() != 1)
                return new BError( "SetSInt8 requires 1 parameter." );
            long value;
            if(!ex.getParam( 0, value ))
                return new BError("Invalid parameter");
            s8 _s8=static_cast<s8>(value);
            if (!file.Write(_s8))
                return new BError("Failed to write");
            return new BLong(1);
        }
    case MTH_SETSTRING:
        {
            if(ex.numParams() != 2)
                return new BError( "SetString requires 2 parameters." );
            long value;
            const String* text;
            if((!ex.getStringParam( 0, text )) ||
                (!ex.getParam( 1, value )) )
                return new BError( "Invalid parameter" );
            int len=text->value().length();
            if (value==1)
                len++;
            if (!file.WriteString(text->value().c_str(),len))
                return new BError("Failed to write");
            return new BLong(1);
        }

    default:
        return NULL;
    }
}

BObjectImp* BBinaryfile::copy() const
{
    return new BBinaryfile(_filename,static_cast<unsigned short>(openmode),bigendian);
}

std::string BBinaryfile::getStringRep() const
{
    return "BinaryFile";
}

bool BBinaryfile::isTrue() const
{
	if (_filename=="")
		return false;
	return file.IsOpen();
}

bool BBinaryfile::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTBinaryFile))
	{
		if (((BBinaryfile&)objimp)._filename==_filename)
			return true;
	}
	return false;
}


BinFile::BinFile()
{
}

BinFile::BinFile( const string& filename, ios::openmode mode )
{
    Open( filename, mode );
}

BinFile::~BinFile()
{
    Close();
}

bool BinFile::Open( const string& filename, ios::openmode mode )
{
    if ( _file.is_open() )
        return true;

    _file.open( filename.c_str(), mode|ios::binary );
    if (!_file.is_open())
        return false;
    return true;
}

bool BinFile::IsOpen()
{
	return _file.is_open();
}

void BinFile::Close()
{
    if (!_file.is_open() )
        return;

    _file.close();
}

bool BinFile::Seek( fstream::pos_type abs_offset, ios::seekdir origin )
{
    if (!_file.is_open() )
        return false;

    if (!_file.seekg( abs_offset, origin ))
        return false;
    return true;
}

bool BinFile::ReadBuffer( void* buffer, streamsize length )
{
    if (!_file.read( reinterpret_cast<char*>(buffer), length ))
        return false;
    return true;
}

bool BinFile::WriteBuffer( void* buffer, streamsize length )
{
    if (!_file.write(reinterpret_cast<char*>(buffer), length ))
        return false;
    return true;
}

bool BinFile::WriteString(const char* chr, unsigned len)
{
    if (!_file.write(chr, len ))
        return false;
    return true;
}

fstream::pos_type BinFile::FileSize(Executor& exec)
{
    if (!_file.is_open() )
        return fstream::pos_type(0);

    fstream::pos_type save_pos = _file.tellg();
    if (save_pos == fstream::pos_type(-1))
    {
        exec.setFunctionResult( new BError( "FileSize failed to determine current position."  ) );
        return fstream::pos_type(0);
    }
    if (!_file.seekg( 0, ios::end ))
    {
        exec.setFunctionResult( new BError( "FileSize failed to seek to end of file."  ) );
        return fstream::pos_type(0);
    }
    fstream::pos_type size = _file.tellg();
    if (size == fstream::pos_type(-1))
    {
        exec.setFunctionResult( new BError( "FileSize could not determine file size."  ) );
        return fstream::pos_type(0);
    }
    if (!_file.seekg( save_pos ))
    {
        exec.setFunctionResult( new BError( "FileSize failed to seek to original position."  ) );
        return fstream::pos_type(0);
    }
    return size;
}

fstream::pos_type BinFile::Tell()
{
    if (!_file.is_open() )
        return -1;
    return _file.tellg();
}

int BinFile::Peek()
{
    if (!_file.is_open() )
        return -1;
    return _file.peek();
}

void BinFile::Flush()
{
    if (_file.is_open() )
        _file.flush();
}
