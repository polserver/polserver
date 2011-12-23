/*
History
=======
2005/09/12 Shinigami: added ObjMethods packet.GetIntxxFlipped and packet.SetIntxxFlipped (Byte Order)
2006/09/16 Shinigami: added ObjMethods packet.GetUnicodeStringFlipped and packet.SetUnicodeStringFlipped (Byte Order)
2006/09/16 Shinigami: fixed Memory Overwrite Bug in packet.SetUnicodeString* -> convertArrayToUC
2008/12/17 MuadDib:   fixed Memory Leak in SetSize() where it returns BObjects back
                      to calling Methods where they do not handle a return value.
2009/08/25 Shinigami: STLport-5.2.1 fix: oldsize not used
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/endian.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/unicode.h"

#include "../bscript/executor.h"
#include "../bscript/berror.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/clienttransmit.h"
#include "packetscrobj.h"
#include "polsem.h"
#include "realms.h"
#include "uoexhelp.h"
#include "ufunc.h"
#include "uvars.h"
#include "zone.h"
#include "uoscrobj.h"

BPacket::BPacket(): BObjectImp( OTPacket ),
is_variable_length(false)
{
}
BPacket::BPacket(const BPacket& copyfrom): BObjectImp( OTPacket ),
buffer(copyfrom.buffer),
is_variable_length(copyfrom.is_variable_length)
{
}
BPacket::BPacket(u8 type, signed short length): BObjectImp( OTPacket )
{
    if(length == -1)
    {
        is_variable_length = true;
        buffer.resize(1,type);
    }
    else
    {
        is_variable_length = false;
        if(length > 0)
        {
            buffer.resize(length,0);
            buffer[0] = type;
        }
    }
}
BPacket::BPacket(const unsigned char* data, unsigned short length, bool variable_len): BObjectImp( OTPacket ),
buffer( data, data+length )
{
    is_variable_length = variable_len;
}
BPacket::~BPacket()
{
}

BObjectRef BPacket::get_member_id( const int id )//id test
{
    return BObjectRef(new BLong(0));
}
BObjectRef BPacket::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* BPacket::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
    switch(id)
    {
    case MTH_SENDPACKET:
        {
		    if(ex.numParams() != 1)
			    return new BError( "SendPacket requires 1 parameter." );

			Character* chr=NULL;
			Client* client=NULL;
			if (getCharacterOrClientParam(ex, 0, chr, client))
			{
				if (chr!=NULL)
				{
					if(!chr->has_active_client())
						return new BLong( 0 );
					
					client = chr->client;
				}
				
				if (client!=NULL)
				{
					if (client->isConnected())
					{
						ADDTOSENDQUEUE(client,(void*)(&buffer[0]),static_cast<int>(buffer.size()));
						return new BLong( 1 );
					}
					else
						return new BLong( 0 );
				}
				else
					new BError("Invalid parameter");
			}
        }

    case MTH_SENDAREAPACKET:
        {
		    if(ex.numParams() != 4)
			    return new BError( "SendAreaPacket requires 4 parameters." );
            unsigned short x, y, range;
            const String* strrealm;
            if(ex.getParam( 0, x ) &&  
               ex.getParam( 1, y ) &&
               ex.getParam( 2, range ) &&
		       ex.getStringParam(3, strrealm) )
            {
			    Realm* realm = find_realm(strrealm->value());
			    if(!realm) return new BError("Realm not found");
			    if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

                unsigned short num_sent_to = 0;
                for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
                {
                    Client* client = *itr;

                    if(!client->chr || !client->ready)
                        continue;
                    if( (client->chr->realm == realm) && inrangex( client->chr, x, y, range ))
                    {
                        ADDTOSENDQUEUE(client, (void*)(&buffer[0]),static_cast<int>(buffer.size()) );
                        num_sent_to++;
                    }
                }
                return new BLong( num_sent_to );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETINT8:
        {
		    if (ex.numParams() != 1)
			    return new BError( "GetInt8 requires 1 parameter." );
            unsigned short offset;
            if (ex.getParam( 0, offset ))
            {
                if( offset >= buffer.size() ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );
                u8* data = reinterpret_cast<u8*>(&buffer[offset]);
                return new BLong( *data );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETINT16:
        {
		    if(ex.numParams() != 1)
			    return new BError( "GetInt16 requires 1 parameter." );
            unsigned short offset;
            if(ex.getParam( 0, offset ))
            {
                if( offset > buffer.size()-sizeof(u16) ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );
                u16* data = reinterpret_cast<u16*>(&buffer[offset]);
                return new BLong( cfBEu16(*data) );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETINT32:
        {
		    if(ex.numParams() != 1)
			    return new BError( "GetInt32 requires 1 parameter." );
            unsigned short offset;
            if(ex.getParam( 0, offset ))
            {
                if( offset > buffer.size()-sizeof(u32) ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );
                u32* data = reinterpret_cast<u32*>(&buffer[offset]);
                return new BLong( cfBEu32(*data) );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETINT16FLIPPED:
        {
		    if(ex.numParams() != 1)
			    return new BError( "GetInt16Flipped requires 1 parameter." );
            unsigned short offset;
            if(ex.getParam( 0, offset ))
            {
                if( offset > buffer.size()-sizeof(u16) ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );
                u16* data = reinterpret_cast<u16*>(&buffer[offset]);
                return new BLong( cfLEu16(*data) );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETINT32FLIPPED:
        {
		    if(ex.numParams() != 1)
			    return new BError( "GetInt32Flipped requires 1 parameter." );
            unsigned short offset;
            if(ex.getParam( 0, offset ))
            {
                if( offset > buffer.size()-sizeof(u32) ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );
                u32* data = reinterpret_cast<u32*>(&buffer[offset]);
                return new BLong( cfLEu32(*data) );
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETSTRING:
        {
		    if(ex.numParams() != 2)
			    return new BError( "GetString requires 2 parameter." );
            unsigned short offset, len;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, len ))
            {
                if( (offset >= buffer.size()) || (static_cast<u16>(offset+len) > buffer.size())  ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );

			    const char* str_offset = reinterpret_cast<const char*>(&buffer[offset]);
			    int real_len = 0;

			    // Returns maximum of len characters or up to the first null-byte
			    while (real_len < len && *(str_offset+real_len))
				    real_len++;

                return new String( str_offset, real_len);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETUNICODESTRING:
        {
		    if(ex.numParams() != 2)
			    return new BError( "GetUnicodeString requires 2 parameter." );
            unsigned short offset, len;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, len )) //len is in unicode characters, not bytes
            {
                if( (offset >= buffer.size()) || (static_cast<u16>(offset+len*2) > buffer.size())  ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );

                ObjArray* arr;
                convertUCtoArray( reinterpret_cast<u16*>(&buffer[offset]),arr,len,true );
                return arr;
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETUNICODESTRINGFLIPPED:
        {
		    if(ex.numParams() != 2)
			    return new BError( "GetUnicodeStringFlipped requires 2 parameter." );
            unsigned short offset, len;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, len )) //len is in unicode characters, not bytes
            {
                if( (offset >= buffer.size()) || (static_cast<u16>(offset+len*2) > buffer.size())  ) //don't allow getting bytes past end of buffer
                    return new BError( "Offset too high" );

                ObjArray* arr;
                convertUCtoArray( reinterpret_cast<u16*>(&buffer[offset]),arr,len,false );
                return arr;
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_GETSIZE:
        return new BLong(static_cast<int>(buffer.size()));

    case MTH_SETSIZE:
        {
		    if(ex.numParams() != 1)
			    return new BError( "SetSize requires 1 parameter." );
            unsigned short newsize;
            if(ex.getParam( 0, newsize ))
            {
                return SetSize(newsize, true);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETINT8:
        {
		    if(ex.numParams() != 2)
			    return new BError( "SetInt8 requires 2 parameters." );
            unsigned short offset, value;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, value ))
            {
                if(is_variable_length)
			    if( offset >= buffer.size() )
			    {
				    if (!SetSize( (offset+sizeof(u8)) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }
                buffer[offset] = static_cast<u8>(value);
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETINT16:
        {
		    if(ex.numParams() != 2)
			    return new BError( "SetInt16 requires 2 parameters." );
            unsigned short offset, value;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, value ))
            {
                if( static_cast<u16>(offset+sizeof(u16)) > buffer.size() )
			    {
				    if (!SetSize( (offset+sizeof(u16)) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }

                u16* bufptr = reinterpret_cast<u16*>(&buffer[offset]);
                *bufptr = ctBEu16( static_cast<u16>(value) );
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETINT32:
        {
		    if(ex.numParams() != 2)
			    return new BError( "SetInt32 requires 2 parameters." );
            unsigned short offset;
            int lvalue;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, lvalue ))
            {
                if( static_cast<u32>(offset+sizeof(u32)) > buffer.size() )
			    {
				    if (!SetSize( offset+sizeof(u32) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }

                u32* bufptr = reinterpret_cast<u32*>(&buffer[offset]);
                *bufptr = ctBEu32( static_cast<u32>(lvalue) );
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETINT16FLIPPED:
        {
		    if(ex.numParams() != 2)
			    return new BError( "SetInt16Flipped requires 2 parameters." );
            unsigned short offset, value;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, value ))
            {
                if( static_cast<u16>(offset+sizeof(u16)) > buffer.size() )
			    {
				    if (!SetSize( (offset+sizeof(u16)) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }

                u16* bufptr = reinterpret_cast<u16*>(&buffer[offset]);
                *bufptr = ctLEu16( static_cast<u16>(value) );
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETINT32FLIPPED:
        {
		    if(ex.numParams() != 2)
			    return new BError( "SetInt32Flipped requires 2 parameters." );
            unsigned short offset;
            int lvalue;
            if(ex.getParam( 0, offset ) &&
               ex.getParam( 1, lvalue ))
            {
                if( static_cast<u32>(offset+sizeof(u32)) > buffer.size() )
			    {
				    if (!SetSize( (offset+sizeof(u32)) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }
                u32* bufptr = reinterpret_cast<u32*>(&buffer[offset]);
                *bufptr = ctLEu32( static_cast<u32>(lvalue) );
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETSTRING:
        {
		    if(ex.numParams() != 3)
			    return new BError( "SetString requires 3 parameters." );
            unsigned short offset, nullterm;
            const String* text;
            if(ex.getParam( 0, offset ) &&
               ex.getStringParam( 1, text ) &&
               ex.getParam( 2, nullterm ) )
            {
                u16 textlen = static_cast<u16>(text->length());
                if( static_cast<u16>(offset+textlen+nullterm) > buffer.size() )
			    {
				    if (!SetSize( (offset+textlen+nullterm) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }

			    u8* bufptr = reinterpret_cast<u8*>(&buffer[offset]);
                const char* textptr = text->value().c_str();
                for(u16 i = 0; i < textlen; i++)
                    bufptr[i] = textptr[i];

                if(nullterm)
                    bufptr[textlen] = 0;

                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter" );
        }

    case MTH_SETUNICODESTRING:
        {
		    if(ex.numParams() != 3)
			    return new BError( "SetUnicodeString requires 3 parameters." );
            unsigned short offset, nullterm;
            ObjArray* unitext;
            if(ex.getParam( 0, offset ) &&
               ex.getObjArrayParam( 1, unitext ) &&
               ex.getParam( 2, nullterm ) )
            {
                u16 textlen = static_cast<u16>(unitext->ref_arr.size()); //number of unicode chars, not bytes
                u16 nulltermlen = nullterm ? 2 : 0;
                if( static_cast<u16>(offset+(textlen * 2)+nulltermlen) > buffer.size() )
			    {
				    if (!SetSize( (offset+(textlen * 2)+nulltermlen) ))
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }
                convertArrayToUC( unitext,reinterpret_cast<u16*>(&buffer[offset]),textlen,true, nullterm ? true : false );

                return new BLong(1);
            }
            else
                return new BError("Invalid parameter");
        }

    case MTH_SETUNICODESTRINGFLIPPED:
        {
		    if(ex.numParams() != 3)
			    return new BError( "SetUnicodeStringFlipped requires 3 parameters." );
            unsigned short offset, nullterm;
            ObjArray* unitext;
            if(ex.getParam( 0, offset ) &&
               ex.getObjArrayParam( 1, unitext ) &&
               ex.getParam( 2, nullterm ) )
            {
                u16 textlen = static_cast<u16>(unitext->ref_arr.size()); //number of unicode chars, not bytes
                u16 nulltermlen = nullterm ? 2 : 0;
                if( static_cast<u16>(offset+(textlen * 2)+nulltermlen) > buffer.size() )
			    {            
				    if (!SetSize( (offset+(textlen * 2)+nulltermlen)) )
				    {
					    return new BError("Offset value out of range on a fixed length packet");					;
				    }
			    }
                convertArrayToUC( unitext,reinterpret_cast<u16*>(&buffer[offset]),textlen,false, nullterm ? true : false );

                return new BLong(1);
            }
            else
                return new BError("Invalid parameter");
        }

    default:
        return NULL;
    }
}


BObjectImp* BPacket::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
	/*
    if(stricmp( methodname, "SendPacket" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "SendPacket requires 1 parameter." );
        Character* chr;
        if(getCharacterParam( ex, 0, chr ))
        {
            if(chr->has_active_client())
            {
                chr->client->transmit((void*)(&buffer[0]),buffer.size());
                return new BLong( 1 );
            }
            else
                return new BLong( 0 );
        }
        else
            return new BError("Invalid parameter");
    }
    if(stricmp( methodname, "SendAreaPacket" ) == 0)
    {
		if(ex.numParams() != 4)
			return new BError( "SendAreaPacket requires 4 parameters." );
        unsigned short x, y, range;
		const String* strrealm;
        if(ex.getParam( 0, x ) &&  
           ex.getParam( 1, y ) &&
           ex.getParam( 2, range ) &&
		   ex.getStringParam( 3, strrealm) )
        {
			Realm* realm = find_realm(strrealm->value());
			if(!realm) return new BError("Realm not found");
			if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

            unsigned short num_sent_to = 0;
            for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
            {
                Client* client = *itr;

                if(!client->chr || !client->ready)
                    continue;
                if( (client->chr->realm == realm) && inrangex( client->chr, x, y, range ))
                {
                    client->transmit( (void*)(&buffer[0]),buffer.size() );
                    num_sent_to++;
                }
            }
            return new BLong( num_sent_to );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetInt8" ) == 0)
    {
		if (ex.numParams() != 1)
			return new BError( "GetInt8 requires 1 parameter." );
        unsigned short offset;
        if (ex.getParam( 0, offset ))
        {
            if( offset >= buffer.size() ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );
            u8* data = reinterpret_cast<u8*>(&buffer[offset]);
            return new BLong( *data );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetInt16" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "GetInt16 requires 1 parameter." );
        unsigned short offset;
        if(ex.getParam( 0, offset ))
        {
            if( offset > buffer.size()-sizeof(u16) ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );
            u16* data = reinterpret_cast<u16*>(&buffer[offset]);
            return new BLong( cfBEu16(*data) );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetInt32" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "GetInt32 requires 1 parameter." );
        unsigned short offset;
        if(ex.getParam( 0, offset ))
        {
            if( offset > buffer.size()-sizeof(u32) ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );
            u32* data = reinterpret_cast<u32*>(&buffer[offset]);
            return new BLong( cfBEu32(*data) );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetInt16Flipped" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "GetInt16Flipped requires 1 parameter." );
        unsigned short offset;
        if(ex.getParam( 0, offset ))
        {
            if( offset > buffer.size()-sizeof(u16) ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );
            u16* data = reinterpret_cast<u16*>(&buffer[offset]);
            return new BLong( cfLEu16(*data) );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetInt32Flipped" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "GetInt32Flipped requires 1 parameter." );
        unsigned short offset;
        if(ex.getParam( 0, offset ))
        {
            if( offset > buffer.size()-sizeof(u32) ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );
            u32* data = reinterpret_cast<u32*>(&buffer[offset]);
            return new BLong( cfLEu32(*data) );
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetString" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "GetString requires 2 parameter." );
        unsigned short offset,len;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, len ))
        {
            if( (offset >= buffer.size()) || (static_cast<u16>(offset+len) > buffer.size())  ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );

            return new String( reinterpret_cast<const char*>(&buffer[offset]), len);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetUnicodeString" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "GetString requires 2 parameter." );
        unsigned short offset,len;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, len )) //len is in unicode characters, not bytes
        {
            if( (offset >= buffer.size()) || (static_cast<u16>(offset+len*2) > buffer.size())  ) //don't allow getting bytes past end of buffer
                return new BError( "Offset too high" );

            ObjArray* arr;
            convertUCtoArray( reinterpret_cast<u16*>(&buffer[offset]),arr,len,true );
            return arr;
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "GetSize" ) == 0)
    {
        return new BLong(buffer.size());
    }
    else if(stricmp( methodname, "SetSize" ) == 0)
    {
		if(ex.numParams() != 1)
			return new BError( "SetSize requires 1 parameter." );
        unsigned short newsize;
        if(ex.getParam( 0, newsize ))
        {
            return SetSize(newsize);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetInt8" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "SetInt8 requires 2 parameters." );
        unsigned short offset,value;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, value ))
        {
            if( offset >= buffer.size() )
                SetSize(offset+sizeof(u8)); //resize to allow data at this offset

            buffer[offset] = static_cast<u8>(value);
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetInt16" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "SetInt16 requires 2 parameters." );
        unsigned short offset,value;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, value ))
        {
            if( static_cast<u16>(offset+sizeof(u16)) > buffer.size() )
                SetSize( offset+sizeof(u16) ); //resize to allow data at this offset

            u16* bufptr = reinterpret_cast<u16*>(&buffer[offset]);
            *bufptr = ctBEu16( static_cast<u16>(value) );
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetInt32" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "SetInt32 requires 2 parameters." );
        unsigned short offset;
        int value;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, value ))
        {
            if( static_cast<u32>(offset+sizeof(u32)) > buffer.size() )
                SetSize( offset+sizeof(u32) ); //resize to allow data at this offset

            u32* bufptr = reinterpret_cast<u32*>(&buffer[offset]);
            *bufptr = ctBEu32( static_cast<u32>(value) );
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetInt16Flipped" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "SetInt16Flipped requires 2 parameters." );
        unsigned short offset,value;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, value ))
        {
            if( static_cast<u16>(offset+sizeof(u16)) > buffer.size() )
                SetSize( offset+sizeof(u16) ); //resize to allow data at this offset

            u16* bufptr = reinterpret_cast<u16*>(&buffer[offset]);
            *bufptr = ctLEu16( static_cast<u16>(value) );
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetInt32Flipped" ) == 0)
    {
		if(ex.numParams() != 2)
			return new BError( "SetInt32Flipped requires 2 parameters." );
        unsigned short offset;
        int value;
        if(ex.getParam( 0, offset ) &&
           ex.getParam( 1, value ))
        {
            if( static_cast<u32>(offset+sizeof(u32)) > buffer.size() )
                SetSize( offset+sizeof(u32) ); //resize to allow data at this offset

            u32* bufptr = reinterpret_cast<u32*>(&buffer[offset]);
            *bufptr = ctLEu32( static_cast<u32>(value) );
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetString" ) == 0)
    {
		if(ex.numParams() != 3)
			return new BError( "SetString requires 3 parameters." );
        u16 offset, nullterm;
        const String* text;

        if(ex.getParam( 0, offset ) &&
           ex.getStringParam( 1, text ) &&
           ex.getParam( 2, nullterm ) )
        {
            u16 textlen = text->length();
            if( static_cast<u16>(offset+textlen+nullterm) > buffer.size() )
                SetSize( offset+textlen+nullterm );

            u8* bufptr = reinterpret_cast<u8*>(&buffer[offset]);
            const char* textptr = text->value().c_str();
            for(u16 i = 0; i < textlen; i++)
                bufptr[i] = textptr[i];

            if(nullterm)
                bufptr[textlen] = 0;

            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter" );
    }
    else if(stricmp( methodname, "SetUnicodeString" ) == 0)
    {
		if(ex.numParams() != 3)
			return new BError( "SetUnicodeString requires 3 parameters." );
        u16 offset, nullterm;
        ObjArray* text;

        if(ex.getParam( 0, offset ) &&
           ex.getObjArrayParam( 1, text ) &&
           ex.getParam( 2, nullterm ) )
        {
            u16 textlen = text->ref_arr.size(); //number of unicode chars, not bytes
            u16 nulltermlen = nullterm ? 2 : 0;
            if( static_cast<u16>(offset+(textlen * 2)+nulltermlen) > buffer.size() )
                SetSize(offset+(textlen * 2)+nulltermlen);

            convertArrayToUC( text,reinterpret_cast<u16*>(&buffer[offset]),textlen,true, nullterm ? true : false );

            return new BLong(1);
        }
        else
            return new BError("Invalid parameter");
    }
    return NULL;
	*/
}
BObjectImp* BPacket::copy() const
{
    return new BPacket(*this);
}
std::string BPacket::getStringRep() const
{
    OSTRINGSTREAM os;
    vector<unsigned char>::const_iterator itr;
    for(itr = buffer.begin(); itr != buffer.end(); ++itr)
        os << std::setfill('0') << std::setw(2) <<  hex << static_cast<u16>(*itr);

    return OSTRINGSTREAM_STR(os);

}
bool BPacket::SetSize(u16 newsize)
{
    if(!is_variable_length)
        return false;
    //unsigned short oldsize = buffer.size();
    buffer.resize(newsize);
    u16* sizeptr = reinterpret_cast<u16*>(&buffer[1]);
    *sizeptr = ctBEu16(newsize);
    return true;
}

BObjectImp* BPacket::SetSize(u16 newsize, bool giveReturn)
{
    if(!is_variable_length)
        return new BError("Attempted to resize a fixed length packet");
    unsigned short oldsize = static_cast<unsigned short>(buffer.size());
    buffer.resize(newsize);
    u16* sizeptr = reinterpret_cast<u16*>(&buffer[1]);
    *sizeptr = ctBEu16(newsize);
    return new BLong(oldsize);
}
