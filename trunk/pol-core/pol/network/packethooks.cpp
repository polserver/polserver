/*
History
=======

2006/05/23 Shinigami: added missing Check to ExportedPacketHookHandler() for
                      missing default_handler in Packets with SubCommands
2007/08/19 Shinigami: fixed Memory Leak in PacketHook functions
2009/08/03 MuadDib:   Renaming of MSG_HANDLER_6017 and related, to MSG_HANDLER_V2 for better description
                      Renamed secondary handler class to *_V2 for naming convention
2009/08/25 Shinigami: STLport-5.2.1 fix: params in call of Log2()
2009/09/03 MuadDib:   Relocation of account related cpp/h

Notes
=======
Version member: Positive Integer. This is used to translate the "version" of the packet structure
to the correct internal core Message Handler (Default 1, which translates to use handler[]). Each
new Handler added to the core needs a new Version number here. As of 8/3/09 there is only 2.

*/

#include "../../clib/stl_inc.h"

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/fileutil.h"
#include "../../clib/strutil.h"

#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"


#include "../../plib/pkg.h"

#include "../accounts/account.h"
#include "../mobile/charactr.h"
#include "client.h"
#include "../msghandl.h"
#include "packethooks.h"
#include "../packetscrobj.h"
#include "../uoscrobj.h"
#include "../polsem.h"
namespace Pol {
  namespace Network {
	//stores information about each packet and its script & default handler
	std::vector<PacketHookData> packet_hook_data( 256 );
	std::vector<PacketHookData> packet_hook_data_v2( 256 );
	std::vector<PacketHookData> packet_hook_data_v3( 256 );

	u32 GetSubCmd( const unsigned char* message, PacketHookData* phd )
	{
	  if ( phd->sub_command_length == 1 )
		return *( reinterpret_cast<const u8*>( &message[phd->sub_command_offset] ) );
	  else if ( phd->sub_command_length == 2 )
		return cfBEu16( *( reinterpret_cast<const u16*>( &message[phd->sub_command_offset] ) ) );
	  //else if(phd->sub_command_length == 4)
	  //    return cfBEu32(*(reinterpret_cast<const u32*>(&message[phd->sub_command_offset])));
	  else
		return cfBEu32( *( reinterpret_cast<const u32*>( &message[phd->sub_command_offset] ) ) );

	}

	//MSG_HANDLER function used for each hooked packet type.
	void ExportedPacketHookHandler( Client* client, void* data )
	{
	  //find the script handler data
	  unsigned char* message = static_cast<unsigned char*>( data );
	  PacketHookData* phd = &( packet_hook_data_v2.at( message[0] ) );
	  if ( phd->version == 2 )
	  {
		if ( !CompareVersionDetail( client->getversiondetail(), phd->client_ver ) )
		{
		  phd = &( packet_hook_data.at( message[0] ) );
		}
	  }
	  else
	  {
		phd = &( packet_hook_data.at( message[0] ) );
	  }

	  if ( phd->function == NULL && phd->SubCommands.empty() )
	  {
		if ( phd->default_handler == NULL )
          POLLOG.Format( "Expected packet hook function for msg 0x{:X} but was null!\n")<< (int)*message;
		else  // only SendFunction is definied but default_handler is definied
		  phd->default_handler( client, data );
		return;
	  }

	  if ( !phd->SubCommands.empty() )
	  {
		u32 subcmd = GetSubCmd( message, phd );//cfBEu16(*(reinterpret_cast<u16*>(&message[phd->sub_command_offset])));
		map<u32, PacketHookData*>::iterator itr;
		itr = phd->SubCommands.find( subcmd );
		if ( itr != phd->SubCommands.end() )
		{
		  if ( itr->second->function != NULL )
			phd = itr->second;
		}
	  }
	  if ( phd->function == NULL ) //this will happen if the main packet entry does not define a receive function,
		//but has subcommands, and we've received an unhooked subcmd.
	  {
		if ( phd->default_handler != NULL )
		  phd->default_handler( client, data );
		return;
	  }

	  // Sends a client reference if still connecting
	  Bscript::BObjectImp* calling_ref;
	  if ( client->chr )
		calling_ref = client->chr->make_ref();
	  else
		calling_ref = client->make_ref();

	  //This packet has fixed length
	  if ( phd->length != 0 )
	  {
        ref_ptr<Core::BPacket> pkt( new Core::BPacket( message, static_cast<unsigned short>( phd->length ), false ) );
		//if function returns 0, we need to call the default handler


		if ( phd->function->call( calling_ref, pkt.get() ) == 0 )
		{
		  if ( phd->default_handler != NULL )
			phd->default_handler( client, static_cast<void*>( &pkt->buffer[0] ) );
		}
	  }
	  else //packet is variable length
	  {
		//discover packet length, and create new packet
		unsigned short len = cfBEu16( *( reinterpret_cast<unsigned short*>( &message[1] ) ) );
        ref_ptr<Core::BPacket> pkt( new Core::BPacket( message, len, true ) );
		//if function returns 0, we need to call the default handler

		if ( phd->function->call( calling_ref, pkt.get() ) == 0 )
		{
		  if ( phd->default_handler != NULL )
		  {
			//the buffer size may have changed in the script, make sure the packet gets the right size
			//u16* sizeptr = (u16*)(&pkt->buffer[1]);
			//*sizeptr = ctBEu16(pkt->buffer.size());
			phd->default_handler( client, static_cast<void*>( &pkt->buffer[0] ) );
		  }
		}
	  }
	}


    void CallOutgoingPacketExportedFunction( Client* client, const void*& data, int& inlength, ref_ptr<Core::BPacket>& outpacket, PacketHookData* phd, bool& handled )
	{
	  const unsigned char* message = static_cast<const unsigned char*>( data );

	  // Sends a client reference if still connecting
      Bscript::BObjectImp* calling_ref;
	  if ( client->chr )
		calling_ref = client->chr->make_ref();
	  else
		calling_ref = client->make_ref();

	  //This packet has fixed length
	  if ( phd->length != 0 )
	  {
        outpacket.set( new Core::BPacket( message, static_cast<unsigned short>( phd->length ), false ) );
		//if function returns 0, we need to call the default handler

		if ( phd->outgoing_function->call( calling_ref, outpacket.get() ) == 0 )
		{
		  data = static_cast<void*>( &outpacket->buffer[0] );
		  //a fixed-length packet
		  inlength = phd->length;
		  handled = false;
		}
		else
		  handled = true;
	  }
	  else //packet is variable length
	  {
		//discover packet length, and create new packet
		unsigned short len = cfBEu16( *( reinterpret_cast<const unsigned short*>( &message[1] ) ) );
        outpacket.set( new Core::BPacket( message, len, true ) );
		//if function returns 0, we need to call the default handler

		if ( phd->outgoing_function->call( calling_ref, outpacket.get() ) == 0 )
		{
		  //the buffer size may have changed in the script, make sure the packet gets the right size

		  u16* sizeptr = reinterpret_cast<u16*>( &outpacket->buffer[1] ); //var-length packets always have length at 2nd and 3rd byte
		  //*sizeptr = ctBEu16(outpacket->buffer.size());

		  data = static_cast<void*>( &outpacket->buffer[0] );
		  //pass the new size back to client::transmit
		  inlength = cfBEu16( *sizeptr );
		  handled = false;
		}
		else
		  handled = true;
	  }
	}

	bool GetAndCheckPacketHooked( Client* client, const void*& data, PacketHookData*& phd )
	{
	  //find the script handler data
	  bool subcmd_handler_exists = false;
	  const unsigned char* message = static_cast<const unsigned char*>( data );
	  phd = &( packet_hook_data_v2.at( message[0] ) );
	  if ( phd->version == 2 )
	  {
		if ( !CompareVersionDetail( client->getversiondetail(), phd->client_ver ) )
		{
		  phd = &( packet_hook_data.at( message[0] ) );
		}
	  }
	  else
	  {
		phd = &( packet_hook_data.at( message[0] ) );
	  }

	  if ( !phd->SubCommands.empty() )
	  {
		u32 subcmd = GetSubCmd( message, phd );//cfBEu16(*(reinterpret_cast<const u16*>(&message[phd->sub_command_offset])));
		map<u32, PacketHookData*>::iterator itr;
		itr = phd->SubCommands.find( subcmd );
		if ( itr != phd->SubCommands.end() )
		{
		  if ( itr->second->outgoing_function != NULL )
		  {
			phd = itr->second;
			subcmd_handler_exists = true;
		  }
		}
	  }
	  if ( phd->outgoing_function == NULL && !subcmd_handler_exists )
	  {
		return false;
	  }
	  return true;
	}

    void load_packet_entries( const Plib::Package* pkg, Clib::ConfigElem& elem )
	{
	  if ( stricmp( elem.type(), "Packet" ) != 0 )
		return;
	  string lengthstr;
	  int length = 0;

	  unsigned short pktversion;
	  string client_string;
	  VersionDetailStruct client_struct;

      Core::ExportedFunction* exfunc = ( Core::ExportedFunction* )NULL;
      Core::ExportedFunction* exoutfunc = ( Core::ExportedFunction* )NULL;
	  if ( elem.has_prop( "ReceiveFunction" ) )
        exfunc = Core::FindExportedFunction( elem, pkg, elem.remove_string( "ReceiveFunction" ), 2, true );
	  if ( elem.has_prop( "SendFunction" ) )
        exoutfunc = Core::FindExportedFunction( elem, pkg, elem.remove_string( "SendFunction" ), 2, true );

	  char *endptr = NULL;
	  unsigned int idlong = strtoul( elem.rest(), &endptr, 0 );
	  if ( ( endptr != NULL ) &&
		   ( *endptr != '\0' ) &&
		   !isspace( *endptr ) )
	  {
		elem.throw_error( "Packet ID not defined or poorly formed" );
	  }
	  if ( idlong > 0xFF )
		elem.throw_error( "Packet ID must be between 0x0 and 0xFF" );

	  if ( !elem.remove_prop( "Version", &pktversion ) )
		pktversion = 1;
	  client_string = elem.remove_string( "Client", "1.25.25.0" );
	  SetVersionDetailStruct( client_string, client_struct );

	  unsigned char id = static_cast<unsigned char>( idlong );

	  unsigned short subcmdoff;
	  if ( !elem.remove_prop( "SubCommandOffset", &subcmdoff ) )
		subcmdoff = 0;
	  unsigned short subcmdlen;
	  if ( !elem.remove_prop( "SubCommandLength", &subcmdlen ) )
		subcmdlen = 0;

	  if ( elem.remove_prop( "Length", &lengthstr ) )
	  {
		if ( lengthstr == "variable" )
		  length = 0;
		else
		{
		  unsigned short temp;
		  endptr = NULL;
		  temp = (unsigned short)strtoul( lengthstr.c_str(), &endptr, 0 );
		  if ( ( endptr != NULL ) &&
			   ( *endptr != '\0' ) &&
			   !isspace( *endptr ) )
		  {
			elem.throw_error( "Length must be an integer or 'variable'" );
		  }
		  else
			length = temp;
		}

	  }
	  else
		elem.throw_error( "Length property missing." );

      Core::ExportedFunction* existing_in_func = NULL;
      Core::ExportedFunction* existing_out_func = NULL;

	  switch ( pktversion )
	  {
		case 1:
		{
				PacketHookData *hook_data = &packet_hook_data.at( id );
				existing_in_func = hook_data->function;
				existing_out_func = hook_data->outgoing_function;
				break;
		}
		case 2:
		{
				PacketHookData *hook_data = &packet_hook_data_v2.at( id );
				existing_in_func = hook_data->function;
				existing_out_func = hook_data->outgoing_function;
				break;
		}
        case 3: INFO_PRINT << "Packethook Packet Version 3 not implemented\n"; return; break;
        default: INFO_PRINT << "Invalid Packethook Packet Version.\n"; return; break;
	  }

      if ( existing_in_func != NULL )
        POLLOG.Format( "Packet hook receive function multiply defined for packet 0x{:X}!\n" ) << (int)id;
      if ( existing_out_func != NULL )
        POLLOG.Format( "Packet hook send function multiply defined for packet 0x{:X}!\n" ) << (int)id;

	  switch ( pktversion )
	  {
		case 1:
		{
				PacketHookData* pkt_data = &packet_hook_data.at( id );
				pkt_data->function = exfunc;
				pkt_data->outgoing_function = exoutfunc;
				pkt_data->length = length;
				pkt_data->sub_command_offset = subcmdoff;
				pkt_data->sub_command_length = subcmdlen;
				pkt_data->version = pktversion;
				pkt_data->client_ver = client_struct;
                if ( Core::handler[id].msglen )
				{
                  pkt_data->default_handler = Core::handler[id].func;
				}
				if ( length == 0 )
				{
                  Core::MessageHandler( id, MSGLEN_2BYTELEN_DATA, ExportedPacketHookHandler );
				}
				else
				{
                  Core::MessageHandler( id, length, ExportedPacketHookHandler );
				}
				break;
		}
		case 2:
		{
				PacketHookData* pkt_data = &packet_hook_data_v2.at( id );
				pkt_data->function = exfunc;
				pkt_data->outgoing_function = exoutfunc;
				pkt_data->length = length;
				pkt_data->sub_command_offset = subcmdoff;
				pkt_data->sub_command_length = subcmdlen;
				pkt_data->version = pktversion;
				pkt_data->client_ver = client_struct;
				if ( Core::handler_v2[id].msglen )
				{
                  pkt_data->default_handler = Core::handler_v2[id].func;
				}
				if ( length == 0 )
				{
                  Core::MessageHandler_V2( id, MSGLEN_2BYTELEN_DATA, ExportedPacketHookHandler );
				}
				else
				{
                  Core::MessageHandler_V2( id, length, ExportedPacketHookHandler );
				}
				break;
		}
        case 3: INFO_PRINT << "Packethook Packet Version 3 not implemented\n"; return; break;
        default: INFO_PRINT << "Invalid Packethook Packet Version.\n"; return; break;
	  }
	}

	void load_subpacket_entries( const Plib::Package* pkg, Clib::ConfigElem& elem )
	{
	  if ( stricmp( elem.type(), "SubPacket" ) != 0 )
		return;
	  Core::ExportedFunction* exfunc = ( Core::ExportedFunction* )NULL;
	  Core::ExportedFunction* exoutfunc = ( Core::ExportedFunction* )NULL;

	  unsigned short pktversion;
	  string client_string;
	  VersionDetailStruct client_struct;

	  if ( elem.has_prop( "ReceiveFunction" ) )
		exfunc = Core::FindExportedFunction( elem, pkg, elem.remove_string( "ReceiveFunction" ), 2, true );
	  if ( elem.has_prop( "SendFunction" ) )
		exoutfunc = Core::FindExportedFunction( elem, pkg, elem.remove_string( "SendFunction" ), 2, true );

	  char *endptr = NULL;
	  unsigned int idlong = strtoul( elem.rest(), &endptr, 0 );
	  if ( ( endptr != NULL ) &&
		   ( *endptr != '\0' ) &&
		   !isspace( *endptr ) )
	  {
		elem.throw_error( "Packet ID not defined or poorly formed" );
	  }
	  if ( idlong > 0xFF )
		elem.throw_error( "Packet ID must be between 0x0 and 0xFF" );

	  unsigned char id = static_cast<unsigned char>( idlong );

	  unsigned short subid = elem.remove_ushort( "SubCommandID" );

	  if ( !elem.remove_prop( "Version", &pktversion ) )
		pktversion = 1;

	  client_string = elem.remove_string( "Client", "1.25.25.0" );
	  SetVersionDetailStruct( client_string, client_struct );

	  PacketHookData* parent = NULL;
	  switch ( pktversion )
	  {
		case 1:	parent = &packet_hook_data.at( id ); break;
		case 2:	parent = &packet_hook_data_v2.at( id ); break;
        case 3: INFO_PRINT << "Packethook Packet Version 3 not implemented\n"; return; break;
        default: INFO_PRINT << "Invalid Packethook Packet Version.\n"; return; break;
	  }

	  //validate that the parent packet has a definition and a SubCommandOffset
	  if ( !parent->sub_command_offset )
		elem.throw_error( string( "Parent packet " + Clib::hexint( id ) + " does not define SubCommandOffset!" ) );
	  if ( !parent->sub_command_length )
		elem.throw_error( string( "Parent packet " + Clib::hexint( id ) + " does not define SubCommandLength" ) );
	  if ( parent->SubCommands.find( subid ) != parent->SubCommands.end() )
		elem.throw_error( string( "SubCommand " + Clib::hexint( subid ) + " for packet " + Clib::hexint( id ) + " multiply defined!" ) );

	  PacketHookData* SubData = new PacketHookData();
	  SubData->function = exfunc;
	  SubData->outgoing_function = exoutfunc;
	  SubData->length = parent->length;
	  SubData->default_handler = parent->default_handler;
	  SubData->version = pktversion;
	  SubData->client_ver = client_struct;

	  parent->SubCommands.insert( make_pair( subid, SubData ) );
	}

	//loads "uopacket.cfg" entries from packages
	void load_packet_hooks()
	{
	  Plib::load_packaged_cfgs( "uopacket.cfg", "packet subpacket", load_packet_entries );
	  Plib::load_packaged_cfgs( "uopacket.cfg", "packet subpacket", load_subpacket_entries );
	}

	PacketHookData::~PacketHookData()
	{
	  map<u32, PacketHookData*>::iterator itr = SubCommands.begin(), end = SubCommands.end();
	  for ( ; itr != end; ++itr )
	  {
		delete itr->second;
	  }
	  if ( function != NULL )
		delete function;
	  if ( outgoing_function != NULL )
		delete outgoing_function;
	}

	void clean_packethooks()
	{
	  packet_hook_data.clear();
	  packet_hook_data_v2.clear();
	  packet_hook_data_v3.clear();
	}

	void SetVersionDetailStruct( const std::string& ver, VersionDetailStruct& detail )
	{
	  try
	  {
		size_t dot1 = ver.find_first_of( '.', 0 );
		size_t dot2 = ver.find_first_of( '.', dot1 + 1 );
		size_t dot3 = ver.find_first_of( '.', dot2 + 1 );
		if ( dot3 == string::npos )  // since 5.0.7 patch is digit
		{
		  dot3 = dot2 + 1;
		  while ( ( dot3 < ver.length() ) && ( isdigit( ver[dot3] ) ) )
		  {
			dot3++;
		  }
		}

		detail.major = atoi( ver.substr( 0, dot1 ).c_str() );
		detail.minor = atoi( ver.substr( dot1 + 1, dot2 - dot1 - 1 ).c_str() );
		detail.rev = atoi( ver.substr( dot2 + 1, dot3 - dot2 - 1 ).c_str() );
		detail.patch = 0;
		if ( dot3 < ver.length() )
		{
		  if ( ( detail.major <= 5 ) && ( detail.minor <= 0 ) && ( detail.rev <= 6 ) )
		  {
			if ( ver[dot3] != ' ' )
			  detail.patch = ( ver[dot3] - 'a' ) + 1;  // char to int
		  }
		  else
			detail.patch = atoi( ver.substr( dot3 + 1, ver.length() - dot3 - 1 ).c_str() );
		}
	  }
	  catch ( ... )
	  {
		detail.major = 0;
		detail.minor = 0;
		detail.rev = 0;
		detail.patch = 0;
        POLLOG_ERROR.Format( "Malformed client version string in Packethook: {}\n" ) << ver;
	  }
	}

	bool CompareVersionDetail( VersionDetailStruct ver1, VersionDetailStruct ver2 )
	{
	  if ( ver1.major > ver2.major )
		return true;
	  else if ( ver1.major < ver2.major )
		return false;
	  else if ( ver1.minor > ver2.minor )
		return true;
	  else if ( ver1.minor < ver2.minor )
		return false;
	  else if ( ver1.rev   > ver2.rev )
		return true;
	  else if ( ver1.rev   < ver2.rev )
		return false;
	  else if ( ver1.patch > ver2.patch )
		return true;
	  else if ( ver1.patch < ver2.patch )
		return false;
	  else
		return true;
	}

  }
}