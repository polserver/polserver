/** @file
 *
 * @par History
 */

#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <zlib.h>

#include "plib/clientfiles/RawMap.h"
#include "plib/clientfiles/uoclientfiles.h"
#include "plib/systemstate.h"
#include "plib/uopreader/uop.h"
#include "plib/uopreader/uophash.h"
#include "plib/ustruct.h"
#include "pol/testing/testenv.h"

// The UOP container reader and the raw map that consumes it. Neither is reachable from the shard
// suite, whose test realms are built from POL's own .mul files rather than from a client install:
// nothing in the tree ships a .uop, so these cases synthesize one byte by byte.

namespace Pol::Testing
{
namespace
{
const std::string testdir = unittest_path( "uop" );

// --- little-endian writers, so the blob below reads as a layout rather than as arithmetic -------

void put_u16( std::string& out, std::uint16_t v )
{
  for ( int i = 0; i < 2; ++i )
    out += static_cast<char>( ( v >> ( 8 * i ) ) & 0xff );
}

void put_u32( std::string& out, std::uint32_t v )
{
  for ( int i = 0; i < 4; ++i )
    out += static_cast<char>( ( v >> ( 8 * i ) ) & 0xff );
}

void put_u64( std::string& out, std::uint64_t v )
{
  for ( int i = 0; i < 8; ++i )
    out += static_cast<char>( ( v >> ( 8 * i ) ) & 0xff );
}

// --- the synthetic map -------------------------------------------------------------------------

// 16x16 tiles: two block columns of two blocks each, split over two UOP chunks of two blocks.
constexpr unsigned short map_width = 16;
constexpr unsigned short map_height = 16;
constexpr size_t blocks_total = 4;
constexpr size_t blocks_per_chunk = 2;
constexpr size_t chunk_bytes = blocks_per_chunk * sizeof( Plib::USTRUCT_MAPINFO_BLOCK );

// Every cell carries its block and its offset within it, so a misplaced block is visible in the
// value rather than only in a byte count.
Plib::USTRUCT_MAPINFO_BLOCK make_block( unsigned int index )
{
  Plib::USTRUCT_MAPINFO_BLOCK block{};
  block.hdr = index;
  for ( unsigned int y = 0; y < 8; ++y )
  {
    for ( unsigned int x = 0; x < 8; ++x )
    {
      block.cell[y][x].landtile = static_cast<u16>( index * 100 + y * 8 + x );
      block.cell[y][x].z = static_cast<s8>( index * 8 + x );
    }
  }
  return block;
}

// The map data of one chunk, as the bytes a .mul would hold.
std::string chunk_payload( size_t chunkidx )
{
  std::string out;
  for ( size_t i = 0; i < blocks_per_chunk; ++i )
  {
    Plib::USTRUCT_MAPINFO_BLOCK block =
        make_block( static_cast<unsigned int>( chunkidx * blocks_per_chunk + i ) );
    out.append( reinterpret_cast<const char*>( &block ), sizeof( block ) );
  }
  return out;
}

// The name a map chunk is filed under inside the container, which is what its hash is taken of.
// The core builds the same name with snprintf( "build/map%dlegacymul/%08i.dat" ); the hash is of
// the bytes, so the two have to agree character for character.
std::string map_chunk_name( int mapid, size_t chunkidx )
{
  return fmt::format( "build/map{}legacymul/{:08}.dat", mapid, chunkidx );
}

struct UopChunk
{
  std::uint64_t filehash;
  std::string payload;
  // The container records a decompressed size separately from the bytes it holds. Unset means the
  // two are the same, which is what an uncompressed chunk looks like.
  std::optional<std::uint32_t> decompressed_size = std::nullopt;
  std::uint16_t compression = uop_t::COMPRESSION_TYPE_NO_COMPRESSION;
};

// A version 5 UOP container with one file per block, so that the block chain has to be walked.
// declared_nfiles is the count the header claims: the reader trusts it to decide when to stop, so
// a count larger than the number of files present is what drives it off the end of the chain.
std::string build_uop( const std::vector<UopChunk>& chunks, std::uint32_t declared_nfiles )
{
  constexpr size_t header_size = 28;
  constexpr size_t block_head_size = 12;  // nfiles + next_addr
  constexpr size_t file_entry_size = 34;

  const size_t nblocks = chunks.size();
  const size_t block_size = block_head_size + file_entry_size;
  const size_t first_block = header_size;
  const size_t first_payload = header_size + nblocks * block_size;

  std::string out;
  out += std::string( "\x4D\x59\x50\x00", 4 );  // "MYP\0"
  put_u32( out, 5 );                            // version
  put_u32( out, 0xFD23EC43 );                   // signature
  put_u64( out, nblocks ? first_block : 0 );    // first block address
  put_u32( out, 1 );                            // max files per block
  put_u32( out, declared_nfiles );

  size_t payload_at = first_payload;
  for ( size_t b = 0; b < nblocks; ++b )
  {
    put_u32( out, 1 );  // one file in this block
    // The last block ends the chain with a zero address.
    put_u64( out, b + 1 < nblocks ? first_block + ( b + 1 ) * block_size : 0 );

    const UopChunk& chunk = chunks[b];
    put_u64( out, payload_at );                                          // data address
    put_u32( out, 0 );                                                   // extra header length
    put_u32( out, static_cast<std::uint32_t>( chunk.payload.size() ) );  // compressed size
    put_u32( out, chunk.decompressed_size.value_or(
                      static_cast<std::uint32_t>( chunk.payload.size() ) ) );
    put_u64( out, chunk.filehash );
    put_u32( out, 0 );  // adler32, unchecked by the reader
    put_u16( out, chunk.compression );

    payload_at += chunk.payload.size();
  }

  for ( const auto& chunk : chunks )
    out += chunk.payload;

  return out;
}

// The container for a map, with one chunk per group of blocks. skip_chunk leaves a hole in the
// chunk numbering, which is what a reader looking for chunk N in order runs into.
std::string build_map_uop( int mapid, std::uint32_t declared_nfiles, bool skip_chunk = false )
{
  std::vector<UopChunk> chunks;
  for ( size_t i = 0; i < blocks_total / blocks_per_chunk; ++i )
  {
    const size_t named = ( skip_chunk && i > 0 ) ? i + 1 : i;
    chunks.push_back( UopChunk{ .filehash = HashLittle2( map_chunk_name( mapid, named ) ),
                                .payload = chunk_payload( i ) } );
  }
  return build_uop( chunks, declared_nfiles );
}

void write_file( const std::string& path, const std::string& bytes )
{
  std::ofstream ofs( path, std::ios::binary );
  ofs.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) );
}

// --- the multi collection ------------------------------------------------------------------------

// One component of a multi as the container stores it, which is not the shape the reader hands
// back: the flag word becomes a single "is static" bit, and the cliloc ids are skipped entirely.
struct MultiElement
{
  u16 graphic;
  s16 x;
  s16 y;
  s16 z;
  std::uint16_t flag;
  std::uint32_t clilocs;
  // Declare the cliloc ids without writing them, which is the only way to reach the reader's
  // bound on them: writing the count it claims would just make the payload that much longer.
  bool omit_clilocs = false;
};

std::string multi_payload( std::uint32_t id, const std::vector<MultiElement>& elems )
{
  std::string out;
  put_u32( out, id );
  put_u32( out, static_cast<std::uint32_t>( elems.size() ) );  // read back as a signed count
  for ( const auto& elem : elems )
  {
    put_u16( out, elem.graphic );
    put_u16( out, static_cast<std::uint16_t>( elem.x ) );
    put_u16( out, static_cast<std::uint16_t>( elem.y ) );
    put_u16( out, static_cast<std::uint16_t>( elem.z ) );
    put_u16( out, elem.flag );
    put_u32( out, elem.clilocs );
    if ( !elem.omit_clilocs )
    {
      for ( std::uint32_t i = 0; i < elem.clilocs; ++i )
        put_u32( out, 1000 + i );
    }
  }
  return out;
}

// A multi is filed under its own id, not under a running chunk number. Same agreement with the
// core's snprintf( "build/multicollection/%06i.bin" ) as above.
std::string multi_name( std::uint32_t id )
{
  return fmt::format( "build/multicollection/{:06}.bin", id );
}

std::string zlib_compressed( const std::string& raw )
{
  unsigned long len = compressBound( static_cast<unsigned long>( raw.size() ) );
  std::string out( len, '\0' );
  compress( reinterpret_cast<unsigned char*>( out.data() ), &len,
            reinterpret_cast<const unsigned char*>( raw.data() ),
            static_cast<unsigned long>( raw.size() ) );
  out.resize( len );
  return out;
}

void write_multi_uop( const std::vector<UopChunk>& chunks, std::uint32_t declared_nfiles )
{
  write_file( testdir + "/MultiCollection.uop", build_uop( chunks, declared_nfiles ) );
}

// --- the hash ----------------------------------------------------------------------------------

// HashLittle2 is a port of Bob Jenkins' lookup3 hashlittle2, and it is what ties a chunk name to
// its entry in the container: a change to it makes every existing client .uop unreadable. The
// values below were cross-checked against an independent implementation of that algorithm rather
// than read back out of this one.
//
// Only ASCII names appear here on purpose. The function accumulates s[k], a plain char, so a byte
// >= 0x80 contributes a negative value where char is signed (x86) and a positive one where it is
// not (ARM). Chunk names are ASCII, so no shipped hash depends on that -- but a case with a high
// byte in it would pass on one platform and fail on the other.
void test_hashlittle2()
{
  // Nothing to mix: a, b and c are all the seed, and only b and c are returned.
  UnitTest( []() { return HashLittle2( "" ); }, static_cast<std::uint64_t>( 0xDEADBEEFDEADBEEFull ),
            "the hash of an empty string is the bare seed" );

  // One case per tail length, which is one switch arm each.
  const char* tails[] = { "a",         "ab",         "abc",         "abcd",
                          "abcde",     "abcdef",     "abcdefg",     "abcdefgh",
                          "abcdefghi", "abcdefghij", "abcdefghijk", "abcdefghijkl" };
  const std::uint64_t expected[] = {
      0x582647AC58D68708ull, 0x6B79A0F2FBB3A8DFull, 0x3C03BE9E0E397631ull, 0xE20DD3FAB5F4889Cull,
      0x03CD18DF026D72DEull, 0x23A820A4D6FA502Eull, 0x59AC1D9DB11AD4A5ull, 0xC79695242995C3BEull,
      0xD99384FFAC6572B4ull, 0x4E0E39008BF7D2EFull, 0x83D518745F61EDF8ull, 0x75B50EC04012F87Bull };
  static_assert( sizeof( tails ) / sizeof( *tails ) == sizeof( expected ) / sizeof( *expected ) );
  for ( size_t i = 0; i < sizeof( tails ) / sizeof( *tails ); ++i )
  {
    UnitTest( [&]() { return HashLittle2( tails[i] ); }, expected[i],
              fmt::format( "the hash of a {}-character name", i + 1 ) );
  }

  // Longer than twelve characters runs the mixing loop as well as the tail.
  UnitTest( []() { return HashLittle2( "build/map0legacymul/00000000.dat" ); },
            static_cast<std::uint64_t>( 0xDBB7AFA433A3764Bull ), "the hash of a map chunk name" );

  // Distinct names hash apart, which is the only property the reader actually relies on.
  UnitTest(
      []()
      {
        return HashLittle2( map_chunk_name( 0, 0 ) ) != HashLittle2( map_chunk_name( 0, 1 ) ) &&
               HashLittle2( map_chunk_name( 0, 0 ) ) != HashLittle2( map_chunk_name( 1, 0 ) );
      },
      true, "chunk names of different maps and indices hash apart" );
}

// --- the container -----------------------------------------------------------------------------

// Walking a container directly, without the map reader on top: the header, the block chain, the
// file entries and the lazily read payload.
void test_uop_container()
{
  const std::string blob = build_map_uop( 0, 2 );

  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        return uop.header()->version() == std::string( "\x05\x00\x00\x00", 4 ) &&
               uop.header()->nfiles() == 2 && uop.header()->maxfiles_per_block() == 1;
      },
      true, "a container reports its header" );

  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        // Two blocks of one file each, and the second ends the chain.
        auto* first = uop.header()->firstblock();
        auto* second = first->block_body()->next_addr();
        return first->block_body()->nfiles() == 1 && first->block_body()->files()->size() == 1 &&
               second->blockaddr() != 0 && second->block_body()->nfiles() == 1 &&
               second->block_body()->next_addr()->blockaddr() == 0;
      },
      true, "a container's blocks form a chain" );

  // A zero address has no body at all, which is how the chain ends. Only _is_null_block_body()
  // may be asked: block_body() itself returns an uninitialised pointer for an absent body, so
  // what it answers is not the reader's to promise. See uop.cpp:40.
  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        auto* last = uop.header()->firstblock()->block_body()->next_addr();
        auto* end = last->block_body()->next_addr();
        return end->blockaddr() == 0 && end->_is_null_block_body();
      },
      true, "the block after the last one has no body" );

  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        auto* file = uop.header()->firstblock()->block_body()->files()->at( 0 );
        return file->compressed_size() == chunk_bytes && file->decompressed_size() == chunk_bytes &&
               file->file_len() == 0 && file->adler32() == 0 &&
               file->compression_type() == uop_t::COMPRESSION_TYPE_NO_COMPRESSION &&
               file->filehash() == HashLittle2( map_chunk_name( 0, 0 ) );
      },
      true, "a file entry reports its sizes and its hash" );

  // The payload is read on demand from the address in the entry, and is the file's own bytes
  // with the extra header split off the front.
  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        auto* file = uop.header()->firstblock()->block_body()->files()->at( 0 );
        return file->data()->filebytes() == chunk_payload( 0 ) && file->data()->bytes().empty() &&
               file->_raw_data().size() == chunk_bytes;
      },
      true, "a file's payload is read from its data address" );

  // Reading it twice answers from the first read rather than seeking again.
  UnitTest(
      [&]()
      {
        std::istringstream iss( blob, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        auto* file = uop.header()->firstblock()->block_body()->files()->at( 0 );
        return file->data() == file->data() && !file->_is_null_data();
      },
      true, "a file's payload is read only once" );

  // An entry with no data address has no payload. Same as the block chain above: the answer is
  // _is_null_data(), never the pointer data() hands back.
  UnitTest(
      []()
      {
        std::string one_empty = build_uop( { UopChunk{ .filehash = 0x1234u, .payload = "" } }, 1 );
        // Blank the data address of the single entry: header (28) + nfiles (4) + next_addr (8).
        for ( size_t i = 0; i < 8; ++i )
          one_empty[28 + 12 + i] = 0;
        std::istringstream iss( one_empty, std::ios::binary );
        kaitai::kstream ks( &iss );
        uop_t uop( &ks );
        auto* file = uop.header()->firstblock()->block_body()->files()->at( 0 );
        return file->dataaddr() == 0 && file->_is_null_data();
      },
      true, "a file entry with no data address has no payload" );

  UnitTest(
      []()
      {
        std::string bad = build_map_uop( 0, 2 );
        bad[1] = 'X';  // "MXP\0" is not the container magic
        try
        {
          std::istringstream iss( bad, std::ios::binary );
          kaitai::kstream ks( &iss );
          uop_t uop( &ks );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a container without the magic is refused" );
}

// --- the raw map -------------------------------------------------------------------------------

// The client data root is a global, and every file helper reads it. Point it at this test's
// directory for the cases below and put it back afterwards.
struct DataRootGuard
{
  DataRootGuard() : saved( Plib::systemstate.config.uo_datafile_root )
  {
    Plib::systemstate.config.uo_datafile_root = testdir + "/";
  }
  ~DataRootGuard() { Plib::systemstate.config.uo_datafile_root = saved; }
  std::string saved;
};

// What every cell of the loaded map should hold, given the block layout the map file was written
// in: block index is column-major, and the cell offsets are the low three bits of each coordinate.
bool cells_match( const Plib::RawMap& map )
{
  const unsigned int hblocks = map_height / 8;
  for ( unsigned short y = 0; y < map_height; ++y )
  {
    for ( unsigned short x = 0; x < map_width; ++x )
    {
      Plib::USTRUCT_MAPINFO cell{};
      const signed char z = map.rawinfo( x, y, &cell );
      const unsigned int block = ( x / 8 ) * hblocks + ( y / 8 );
      const Plib::USTRUCT_MAPINFO_BLOCK expected = make_block( block );
      if ( cell.landtile != expected.cell[y & 0x7][x & 0x7].landtile )
        return false;
      if ( z != expected.cell[y & 0x7][x & 0x7].z || z != cell.z )
        return false;
    }
  }
  return true;
}

void test_rawmap_uop()
{
  // The whole map, read out of a container. open() reports the size a .mul of the same map would
  // have, which is what the caller sizes its realm from.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map9LegacyMUL.uop", build_map_uop( 9, 2 ) );
        Plib::RawMap map;
        const size_t size = map.open( 9, false, true );
        map.full_read( 9, map_width, map_height );
        return size == blocks_total * sizeof( Plib::USTRUCT_MAPINFO_BLOCK ) && map.loaded() &&
               cells_match( map );
      },
      true, "a map read from a container has every block in place" );

  // A header claiming more files than the chain holds runs the walk to the end of the chain and
  // says so, rather than reading past it.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map8LegacyMUL.uop", build_map_uop( 8, 3 ) );
        Plib::RawMap map;
        return map.open( 8, false, true ) == blocks_total * sizeof( Plib::USTRUCT_MAPINFO_BLOCK );
      },
      true, "a container claiming more files than it has is still sized" );

  // Chunk N is looked up by name, so a gap in the numbering is a missing chunk however many
  // files the container holds.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map7LegacyMUL.uop", build_map_uop( 7, 2, true ) );
        Plib::RawMap map;
        try
        {
          map.open( 7, false, true );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a container with a gap in its chunk numbering is refused" );

  // With no container present the reader falls back to the mul, which is the same map written
  // without the container around it.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map6.mul", chunk_payload( 0 ) + chunk_payload( 1 ) );
        Plib::RawMap map;
        const size_t size = map.open( 6, false, true );
        map.full_read( 6, map_width, map_height );
        return size == blocks_total * sizeof( Plib::USTRUCT_MAPINFO_BLOCK ) && cells_match( map );
      },
      true, "a missing container falls back to the mul" );
}

// The dif files patch individual blocks: mapdifl lists the block numbers, mapdif holds the
// replacement blocks in the same order.
void test_rawmap_dif()
{
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map5.mul", chunk_payload( 0 ) + chunk_payload( 1 ) );

        // Block 2 is patched, and its replacement is the block numbered 42.
        std::string difl;
        put_u32( difl, 2 );
        write_file( testdir + "/mapdifl5.mul", difl );

        const Plib::USTRUCT_MAPINFO_BLOCK patch = make_block( 42 );
        write_file( testdir + "/mapdif5.mul",
                    std::string( reinterpret_cast<const char*>( &patch ), sizeof( patch ) ) );

        Plib::RawMap map;
        map.open( 5, true, false );
        if ( map.read_difflist() != 1 || map.num_patches() != 1 )
          return false;
        map.full_read( 5, map_width, map_height );

        // Block 2 is the (8..15, 0..7) corner, and it now reads as the patch.
        Plib::USTRUCT_MAPINFO cell{};
        map.rawinfo( 8, 0, &cell );
        const Plib::USTRUCT_MAPINFO_BLOCK expected = make_block( 42 );
        if ( cell.landtile != expected.cell[0][0].landtile )
          return false;

        // Every unpatched block is untouched.
        map.rawinfo( 0, 0, &cell );
        return cell.landtile == make_block( 0 ).cell[0][0].landtile;
      },
      true, "a dif file replaces the blocks it lists" );

  // Without a mapdifl there is nothing to patch, and the dif files are not even opened.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_file( testdir + "/map4.mul", chunk_payload( 0 ) + chunk_payload( 1 ) );
        Plib::RawMap map;
        map.open( 4, true, false );
        return map.read_difflist() == 0 && map.num_patches() == 0;
      },
      true, "a map with no dif list has no patches" );
}

// --- the multi collection ------------------------------------------------------------------------

// The two components every case below files under multi 0x100: one static, one not, the second
// carrying cliloc ids that the reader has to step over to find the next component.
const std::vector<MultiElement> house_parts = {
    { 0x0064, 1, -2, 3, 0x0000, 0 },  // a zero flag word is a static component
    { 0x0065, -4, 5, -6, 0x0201, 2 } };

bool parts_match( const std::vector<Plib::USTRUCT_MULTI_ELEMENT>& got )
{
  if ( got.size() != house_parts.size() )
    return false;
  for ( size_t i = 0; i < got.size(); ++i )
  {
    const MultiElement& want = house_parts[i];
    // Only the low byte of the flag word is looked at, and only for whether it is zero.
    const u32 flags = ( want.flag & 0xFF ) == 0 ? 1u : 0u;
    if ( got[i].graphic != want.graphic || got[i].x != want.x || got[i].y != want.y ||
         got[i].z != want.z || got[i].flags != flags )
      return false;
  }
  return true;
}

void test_multicollection()
{
  using MultiMap = std::map<unsigned int, std::vector<Plib::USTRUCT_MULTI_ELEMENT>>;

  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                                     .payload = multi_payload( 0x100, house_parts ) },
                           UopChunk{ .filehash = HashLittle2( multi_name( 0x101 ) ),
                                     .payload = multi_payload( 0x101, {} ) } },
                         2 );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) && multis.size() == 2 &&
               parts_match( multis[0x100] ) && multis[0x101].empty();
      },
      true, "a multi collection is read into one entry per multi" );

  // The same collection, stored compressed. The reader inflates it to the recorded decompressed
  // size before reading a single field.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        const std::string raw = multi_payload( 0x100, house_parts );
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                                     .payload = zlib_compressed( raw ),
                                     .decompressed_size = static_cast<std::uint32_t>( raw.size() ),
                                     .compression = uop_t::COMPRESSION_TYPE_ZLIB } },
                         1 );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) && parts_match( multis[0x100] );
      },
      true, "a compressed multi collection is inflated first" );

  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                                     .payload = "not deflate data",
                                     .decompressed_size = 64,
                                     .compression = uop_t::COMPRESSION_TYPE_ZLIB } },
                         1 );
        MultiMap multis;
        try
        {
          Plib::open_uopmulti_file( multis );
        }
        catch ( const std::exception& )
        {
          return true;
        }
        return false;
      },
      true, "a chunk that does not inflate is refused" );

  // The hash is what says a file is a multi at all, so a file the hash does not claim is stepped
  // over rather than read as one.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x999 ) ),
                                     .payload = multi_payload( 0x100, house_parts ) } },
                         1 );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) && multis.empty();
      },
      true, "a file whose hash does not name its multi is skipped" );

  // A chunk with nothing in it is skipped before its payload is even looked at.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                                     .payload = multi_payload( 0x100, house_parts ),
                                     .decompressed_size = 0 } },
                         1 );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) && multis.empty();
      },
      true, "a chunk with no content is skipped" );

  // Both of the reader's own bounds checks: one on a field that runs off the end, one on the
  // cliloc ids a component claims to carry.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        std::string truncated = multi_payload( 0x100, house_parts );
        truncated.resize( truncated.size() - 4 );
        write_multi_uop(
            { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ), .payload = truncated } },
            1 );
        MultiMap multis;
        try
        {
          Plib::open_uopmulti_file( multis );
        }
        catch ( const std::out_of_range& )
        {
          return true;
        }
        return false;
      },
      true, "a payload that ends mid-component is refused" );

  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop(
            { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                        .payload = multi_payload( 0x100, { { .graphic = 0x64,
                                                             .x = 0,
                                                             .y = 0,
                                                             .z = 0,
                                                             .flag = 0,
                                                             .clilocs = 4,
                                                             .omit_clilocs = true } } ) } },
            1 );
        MultiMap multis;
        try
        {
          Plib::open_uopmulti_file( multis );
        }
        catch ( const std::out_of_range& )
        {
          return true;
        }
        return false;
      },
      true, "a component claiming more clilocs than the payload holds is refused" );

  // A header claiming more files than the chain holds walks to the end of the chain and says so.
  UnitTest(
      []()
      {
        DataRootGuard guard;
        write_multi_uop( { UopChunk{ .filehash = HashLittle2( multi_name( 0x100 ) ),
                                     .payload = multi_payload( 0x100, house_parts ) } },
                         2 );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) && parts_match( multis[0x100] );
      },
      true, "a collection claiming more files than it has is still read" );

  UnitTest(
      []()
      {
        DataRootGuard guard;
        std::filesystem::remove( testdir + "/MultiCollection.uop" );
        MultiMap multis;
        return Plib::open_uopmulti_file( multis ) == false && multis.empty();
      },
      true, "a missing multi collection falls back to the mul" );
}
}  // namespace

void uop_test()
{
  UnitTestDir dir( testdir );

  test_hashlittle2();
  test_uop_container();
  test_rawmap_uop();
  test_rawmap_dif();
  test_multicollection();
}
}  // namespace Pol::Testing
