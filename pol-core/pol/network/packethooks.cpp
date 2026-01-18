/** @file
 *
 * @par History
 * - 2006/05/23 Shinigami: added missing Check to ExportedPacketHookHandler() for
 *                         missing default_handler in Packets with SubCommands
 * - 2007/08/19 Shinigami: fixed Memory Leak in PacketHook functions
 * - 2009/08/03 MuadDib:   Renaming of MSG_HANDLER_6017 and related, to MSG_HANDLER_V2 for better
 * description
 *                         Renamed secondary handler class to *_V2 for naming convention
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: params in call of Log2()
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *
 * @note Version member: Positive Integer. This is used to translate the "version" of the packet
 * structure
 * to the correct internal core Message Handler (Default 1, which translates to use handler[]). Each
 * new Handler added to the core needs a new Version number here. As of 8/3/09 there is only 2.
 */


#include "packethooks.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../../clib/cfgelem.h"
#include "../../clib/clib.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../plib/pkg.h"
#include "../globals/network.h"
#include "../mobile/charactr.h"
#include "../packetscrobj.h"
#include "../syshook.h"
#include "client.h"


namespace Pol::Network
{
u32 GetSubCmd( const unsigned char* message, PacketHookData* phd )
{
  if ( phd->sub_command_length == 1 )
    return *( reinterpret_cast<const u8*>( &message[phd->sub_command_offset] ) );
  if ( phd->sub_command_length == 2 )
    return cfBEu16( *( reinterpret_cast<const u16*>( &message[phd->sub_command_offset] ) ) );
  // else if(phd->sub_command_length == 4)
  //    return cfBEu32(*(reinterpret_cast<const u32*>(&message[phd->sub_command_offset])));
  return cfBEu32( *( reinterpret_cast<const u32*>( &message[phd->sub_command_offset] ) ) );
}

// Variable length is defined in MSGLEN_2BYTELEN_DATA
static bool is_fixed_length( const PacketHookData* phd )
{
  return phd->length > 0;
}

// Gets the packet hook for a specific packet version
PacketHookData* get_packethook( u8 msgid, PacketVersion version = PacketVersion::Default )
{
  if ( version == PacketVersion::V2 )
    return Core::networkManager.packet_hook_data_v2.at( msgid ).get();

  return Core::networkManager.packet_hook_data.at( msgid ).get();
}

// Gets the packet hook according to the client version
PacketHookData* get_packethook( u8 msgid, const Client* client )
{
  PacketHookData* phd = get_packethook( msgid, PacketVersion::V2 );
  if ( phd->version == PacketVersion::V2 &&
       CompareVersionDetail( client->getversiondetail(), phd->client_ver ) )
    return phd;

  return get_packethook( msgid, PacketVersion::Default );
}

// MSG_HANDLER function used for each hooked packet type.
void ExportedPacketHookHandler( Client* client, void* data )
{
  // find the script handler data
  unsigned char* message = static_cast<unsigned char*>( data );

  u8 msgid = message[0];
  PacketHookData* phd = get_packethook( msgid, client );

  if ( phd->function == nullptr && phd->SubCommands.empty() )
  {
    if ( phd->default_handler == nullptr )
      POLLOGLN( "Expected packet hook function for msg {:#x} but was null!", (int)*message );
    else  // only SendFunction is definied but default_handler is definied
      phd->default_handler( client, data );
    return;
  }

  if ( !phd->SubCommands.empty() )
  {
    u32 subcmd = GetSubCmd( message, phd );
    std::map<u32, PacketHookData*>::iterator itr;
    itr = phd->SubCommands.find( subcmd );
    if ( itr != phd->SubCommands.end() )
    {
      if ( itr->second->function != nullptr )
        phd = itr->second;
    }
  }
  if ( phd->function ==
       nullptr )  // this will happen if the main packet entry does not define a receive function,
  // but has subcommands, and we've received an unhooked subcmd.
  {
    if ( phd->default_handler != nullptr )
      phd->default_handler( client, data );
    return;
  }

  // Sends a client reference if still connecting
  Bscript::BObjectImp* calling_ref;
  if ( client->chr )
    calling_ref = client->chr->make_ref();
  else
    calling_ref = client->make_ref();

  // This packet has fixed length
  if ( is_fixed_length( phd ) )
  {
    ref_ptr<Core::BPacket> pkt(
        new Core::BPacket( message, static_cast<unsigned short>( phd->length ), false ) );
    // if function returns 0, we need to call the default handler


    if ( phd->function->call( calling_ref, pkt.get() ) == 0 )
    {
      if ( phd->default_handler != nullptr )
        phd->default_handler( client, static_cast<void*>( &pkt->buffer[0] ) );
    }
  }
  else  // packet is variable length
  {
    // discover packet length, and create new packet
    unsigned short len = cfBEu16( *( reinterpret_cast<unsigned short*>( &message[1] ) ) );
    ref_ptr<Core::BPacket> pkt( new Core::BPacket( message, len, true ) );
    // if function returns 0, we need to call the default handler

    if ( phd->function->call( calling_ref, pkt.get() ) == 0 )
    {
      if ( phd->default_handler != nullptr )
      {
        // the buffer size may have changed in the script, make sure the packet gets the right size
        // u16* sizeptr = (u16*)(&pkt->buffer[1]);
        //*sizeptr = ctBEu16(pkt->buffer.size());
        phd->default_handler( client, static_cast<void*>( &pkt->buffer[0] ) );
      }
    }
  }
}


void CallOutgoingPacketExportedFunction( Client* client, const void*& data, int& inlength,
                                         ref_ptr<Core::BPacket>& outpacket, PacketHookData* phd,
                                         bool& handled )
{
  const unsigned char* message = static_cast<const unsigned char*>( data );

  // Sends a client reference if still connecting
  Bscript::BObjectImp* calling_ref;
  if ( client->chr )
    calling_ref = client->chr->make_ref();
  else
    calling_ref = client->make_ref();

  // This packet has fixed length
  if ( is_fixed_length( phd ) )
  {
    outpacket.set(
        new Core::BPacket( message, static_cast<unsigned short>( phd->length ), false ) );
    // if function returns 0, we need to call the default handler

    if ( phd->outgoing_function->call( calling_ref, outpacket.get() ) == 0 )
    {
      data = static_cast<void*>( &outpacket->buffer[0] );
      // a fixed-length packet
      inlength = phd->length;
      handled = false;
    }
    else
      handled = true;
  }
  else  // packet is variable length
  {
    // discover packet length, and create new packet
    unsigned short len = cfBEu16( *( reinterpret_cast<const unsigned short*>( &message[1] ) ) );
    outpacket.set( new Core::BPacket( message, len, true ) );
    // if function returns 0, we need to call the default handler

    if ( phd->outgoing_function->call( calling_ref, outpacket.get() ) == 0 )
    {
      // the buffer size may have changed in the script, make sure the packet gets the right size

      u16* sizeptr = reinterpret_cast<u16*>(
          &outpacket->buffer[1] );  // var-length packets always have length at 2nd and 3rd byte
      //*sizeptr = ctBEu16(outpacket->buffer.size());

      data = static_cast<void*>( &outpacket->buffer[0] );
      // pass the new size back to client::transmit
      inlength = cfBEu16( *sizeptr );

      // This shouldn't trigger unless something is wrong with BPacket.
      passert_r( static_cast<u32>( inlength ) <= outpacket->buffer.size(),
                 "Specified packet length is greater than packet buffer!" );

      handled = false;
    }
    else
      handled = true;
  }
}

bool GetAndCheckPacketHooked( Client* client, const void*& data, PacketHookData*& phd )
{
  // find the script handler data
  bool subcmd_handler_exists = false;
  const unsigned char* message = static_cast<const unsigned char*>( data );

  u8 msgid = message[0];
  phd = get_packethook( msgid, client );

  if ( !phd->SubCommands.empty() )
  {
    u32 subcmd = GetSubCmd( message, phd );
    auto itr = phd->SubCommands.find( subcmd );
    if ( itr != phd->SubCommands.end() )
    {
      if ( itr->second->outgoing_function != nullptr )
      {
        phd = itr->second;
        subcmd_handler_exists = true;
      }
    }
  }
  if ( phd->outgoing_function == nullptr && !subcmd_handler_exists )
  {
    return false;
  }
  return true;
}

static PacketVersion load_packethook_version( Clib::ConfigElem& elem )
{
  unsigned short pktversion;

  if ( !elem.remove_prop( "Version", &pktversion ) )
    pktversion = 1;

  switch ( pktversion )
  {
  case 1:
    return PacketVersion::V1;
  case 2:
    return PacketVersion::V2;
  default:
    elem.throw_error( "Only versions 1 and 2 are currently implemented." );
  }
}

static int load_packethook_length( Clib::ConfigElem& elem )
{
  std::string lengthstr;
  int length = 0;

  if ( !elem.remove_prop( "Length", &lengthstr ) )
    elem.throw_error( "Length property missing." );

  if ( lengthstr == "variable" )
    length = MSGLEN_2BYTELEN_DATA;  // sets length to indicate variable length
  else
  {
    unsigned short temp;
    char* endptr = nullptr;
    temp = (unsigned short)strtoul( lengthstr.c_str(), &endptr, 0 );
    if ( temp == 0 || ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) ) )
    {
      elem.throw_error( "Length must be a positive integer or 'variable'" );
    }
    else
      length = temp;
  }

  return length;
}

static void packethook_warn_if_previously_defined( u8 msgid, PacketVersion pktversion )
{
  PacketHookData* hook_data = get_packethook( msgid, pktversion );

  auto existing_in_func = hook_data->function;
  auto existing_out_func = hook_data->outgoing_function;

  if ( existing_in_func != nullptr )
    POLLOGLN( "Packet hook receive function multiply defined for packet {:#x}!", (int)msgid );
  if ( existing_out_func != nullptr )
    POLLOGLN( "Packet hook send function multiply defined for packet {:#x}!", (int)msgid );
}

void load_packet_entries( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  if ( stricmp( elem.type(), "Packet" ) != 0 )
    return;

  int length = 0;

  PacketVersion pktversion;
  std::string client_string;
  VersionDetailStruct client_struct;

  Core::ExportedFunction* exfunc = (Core::ExportedFunction*)nullptr;
  Core::ExportedFunction* exoutfunc = (Core::ExportedFunction*)nullptr;
  if ( elem.has_prop( "ReceiveFunction" ) )
    exfunc =
        Core::FindExportedFunction( elem, pkg, elem.remove_string( "ReceiveFunction" ), 2, true );
  if ( elem.has_prop( "SendFunction" ) )
    exoutfunc =
        Core::FindExportedFunction( elem, pkg, elem.remove_string( "SendFunction" ), 2, true );

  char* endptr = nullptr;
  unsigned int idlong = strtoul( elem.rest(), &endptr, 0 );
  if ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) )
  {
    elem.throw_error( "Packet ID not defined or poorly formed" );
  }
  if ( idlong > 0xFF )
    elem.throw_error( "Packet ID must be between 0x0 and 0xFF" );

  // Reads the packet version ("Version") and throws an error if not 1 or 2
  pktversion = load_packethook_version( elem );

  client_string = elem.remove_string( "Client", "1.25.25.0" );
  SetVersionDetailStruct( client_string, client_struct );

  unsigned char id = static_cast<unsigned char>( idlong );

  unsigned short subcmdoff;
  if ( !elem.remove_prop( "SubCommandOffset", &subcmdoff ) )
    subcmdoff = 0;
  unsigned short subcmdlen;
  if ( !elem.remove_prop( "SubCommandLength", &subcmdlen ) )
    subcmdlen = 0;

  // Loads the length ("Length"), which is either 'variable' or a positive integer
  // if 'variable', length will be MSGLEN_2BYTELEN_DATA
  length = load_packethook_length( elem );

  // Checks if packethook has been previously defined and prints a warning
  packethook_warn_if_previously_defined( id, pktversion );

  PacketHookData* pkt_data = get_packethook( id, pktversion );
  pkt_data->function = exfunc;
  pkt_data->outgoing_function = exoutfunc;
  pkt_data->length = length;
  pkt_data->sub_command_offset = subcmdoff;
  pkt_data->sub_command_length = subcmdlen;
  pkt_data->version = pktversion;
  pkt_data->client_ver = client_struct;
  pkt_data->default_handler = PacketRegistry::get_callback( id, pktversion );

  PacketRegistry::set_handler( id, length, ExportedPacketHookHandler, pktversion );
}

static void packethook_assert_valid_parent( u8 id, const PacketHookData* parent,
                                            const Clib::ConfigElem& elem )
{
  // validate that the parent packet has a definition and a SubCommandOffset
  if ( !parent->sub_command_offset )
    elem.throw_error( std::string( "Parent packet " + Clib::hexint( id ) +
                                   " does not define SubCommandOffset!" ) );
  if ( !parent->sub_command_length )
    elem.throw_error( std::string( "Parent packet " + Clib::hexint( id ) +
                                   " does not define SubCommandLength" ) );
}

void load_subpacket_entries( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  if ( stricmp( elem.type(), "SubPacket" ) != 0 )
    return;
  Core::ExportedFunction* exfunc = (Core::ExportedFunction*)nullptr;
  Core::ExportedFunction* exoutfunc = (Core::ExportedFunction*)nullptr;

  PacketVersion pktversion;
  std::string client_string;
  VersionDetailStruct client_struct;

  if ( elem.has_prop( "ReceiveFunction" ) )
    exfunc =
        Core::FindExportedFunction( elem, pkg, elem.remove_string( "ReceiveFunction" ), 2, true );
  if ( elem.has_prop( "SendFunction" ) )
    exoutfunc =
        Core::FindExportedFunction( elem, pkg, elem.remove_string( "SendFunction" ), 2, true );

  char* endptr = nullptr;
  unsigned int idlong = strtoul( elem.rest(), &endptr, 0 );
  if ( ( endptr != nullptr ) && ( *endptr != '\0' ) && !isspace( *endptr ) )
  {
    elem.throw_error( "Packet ID not defined or poorly formed" );
  }
  if ( idlong > 0xFF )
    elem.throw_error( "Packet ID must be between 0x0 and 0xFF" );

  unsigned char id = static_cast<unsigned char>( idlong );

  unsigned short subid = elem.remove_ushort( "SubCommandID" );

  // Reads the packet version ("Version") and throws an error if not 1 or 2
  pktversion = load_packethook_version( elem );

  client_string = elem.remove_string( "Client", "1.25.25.0" );
  SetVersionDetailStruct( client_string, client_struct );

  PacketHookData* parent = get_packethook( id, pktversion );
  packethook_assert_valid_parent( id, parent, elem );

  if ( parent->SubCommands.find( subid ) != parent->SubCommands.end() )
    elem.throw_error( std::string( "SubCommand " + Clib::hexint( subid ) + " for packet " +
                                   Clib::hexint( id ) + " multiply defined!" ) );

  PacketHookData* SubData = new PacketHookData();
  SubData->function = exfunc;
  SubData->outgoing_function = exoutfunc;
  SubData->length = parent->length;
  SubData->default_handler = parent->default_handler;
  SubData->version = pktversion;
  SubData->client_ver = client_struct;

  parent->SubCommands.insert( std::make_pair( subid, SubData ) );
}

// loads "uopacket.cfg" entries from packages
void load_packet_hooks()
{
  Plib::load_packaged_cfgs( "uopacket.cfg", "packet subpacket", load_packet_entries );
  Plib::load_packaged_cfgs( "uopacket.cfg", "packet subpacket", load_subpacket_entries );
}

PacketHookData::PacketHookData()
    : length( 0 ),
      function( nullptr ),
      outgoing_function( nullptr ),
      default_handler( nullptr ),
      sub_command_offset( 0 ),
      sub_command_length( 0 ),
      version( PacketVersion::Default )
{
  memset( &client_ver, 0, sizeof( client_ver ) );
};

PacketHookData::~PacketHookData()
{
  std::map<u32, PacketHookData*>::iterator itr = SubCommands.begin(), end = SubCommands.end();
  for ( ; itr != end; ++itr )
  {
    delete itr->second;
  }
  if ( function != nullptr )
    delete function;
  if ( outgoing_function != nullptr )
    delete outgoing_function;
}

void PacketHookData::initializeGameData( std::vector<std::unique_ptr<PacketHookData>>* data )
{
  data->clear();
  data->reserve( 256 );
  for ( int i = 0; i < 256; ++i )
  {
    data->emplace_back( new PacketHookData() );
  }
}

size_t PacketHookData::estimateSize() const
{
  size_t size = sizeof( PacketHookData ) + 2 * sizeof( Core::ExportedFunction );
  size += Clib::memsize( SubCommands );
  for ( const auto& subs : SubCommands )
  {
    if ( subs.second != nullptr )
      size += subs.second->estimateSize();
  }
  return size;
}


void clean_packethooks()
{
  Core::networkManager.packet_hook_data.clear();
  Core::networkManager.packet_hook_data_v2.clear();
}

void SetVersionDetailStruct( const std::string& ver, VersionDetailStruct& detail )
{
  try
  {
    size_t dot1 = ver.find_first_of( '.', 0 );
    size_t dot2 = ver.find_first_of( '.', dot1 + 1 );
    size_t dot3 = ver.find_first_of( '.', dot2 + 1 );
    if ( dot3 == std::string::npos )  // since 5.0.7 patch is digit
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
    POLLOG_ERRORLN( "Malformed client version string in Packethook: {}", ver );
  }
}

bool CompareVersionDetail( VersionDetailStruct ver1, VersionDetailStruct ver2 )
{
  if ( ver1.major > ver2.major )
    return true;
  if ( ver1.major < ver2.major )
    return false;
  if ( ver1.minor > ver2.minor )
    return true;
  if ( ver1.minor < ver2.minor )
    return false;
  if ( ver1.rev > ver2.rev )
    return true;
  if ( ver1.rev < ver2.rev )
    return false;
  if ( ver1.patch > ver2.patch )
    return true;
  if ( ver1.patch < ver2.patch )
    return false;
  return true;
}
}  // namespace Pol::Network
