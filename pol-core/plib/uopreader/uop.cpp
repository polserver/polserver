// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "uop.h"


uop_t::uop_t( kaitai::kstream* p__io, kaitai::kstruct* p__parent, uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = this;
  _read();
}

void uop_t::_read()
{
  m_header = new header_t( m__io, this, m__root );
}

uop_t::~uop_t()
{
  delete m_header;
}

uop_t::block_addr_t::block_addr_t( kaitai::kstream* p__io, kaitai::kstruct* p__parent,
                                   uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = p__root;
  f_block_body = false;
  _read();
}

void uop_t::block_addr_t::_read()
{
  m_blockaddr = m__io->read_u8le();
}

uop_t::block_addr_t::~block_addr_t()
{
  if ( f_block_body && !n_block_body )
  {
    delete m_block_body;
  }
}

uop_t::block_body_t* uop_t::block_addr_t::block_body()
{
  if ( f_block_body )
    return m_block_body;
  n_block_body = true;
  if ( blockaddr() > 0 )
  {
    n_block_body = false;
    std::streampos _pos = m__io->pos();
    m__io->seek( blockaddr() );
    m_block_body = new block_body_t( m__io, this, m__root );
    m__io->seek( _pos );
  }
  f_block_body = true;
  return m_block_body;
}

uop_t::block_body_t::block_body_t( kaitai::kstream* p__io, uop_t::block_addr_t* p__parent,
                                   uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = p__root;
  _read();
}

void uop_t::block_body_t::_read()
{
  m_nfiles = m__io->read_u4le();
  m_next_addr = new block_addr_t( m__io, this, m__root );
  int l_files = nfiles();
  m_files = new std::vector<file_t*>();
  m_files->reserve( l_files );
  for ( int i = 0; i < l_files; i++ )
  {
    m_files->push_back( new file_t( m__io, this, m__root ) );
  }
}

uop_t::block_body_t::~block_body_t()
{
  delete m_next_addr;
  for ( auto& m_file : *m_files )
  {
    delete m_file;
  }
  delete m_files;
}

uop_t::data_header_v5_t::data_header_v5_t( kaitai::kstream* p__io, uop_t::file_t* p__parent,
                                           uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = p__root;
  _read();
}

void uop_t::data_header_v5_t::_read()
{
  m_bytes = m__io->read_bytes( _parent()->file_len() );
  m_filebytes = m__io->read_bytes( _parent()->compressed_size() );
}

uop_t::data_header_v5_t::~data_header_v5_t() = default;

uop_t::header_t::header_t( kaitai::kstream* p__io, uop_t* p__parent, uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = p__root;
  _read();
}

void uop_t::header_t::_read()
{
  m_magic = m__io->ensure_fixed_contents( std::string( "\x4D\x59\x50\x00", 4 ) );
  m_version = m__io->read_bytes( 4 );
  m_signature = m__io->read_bytes( 4 );
  m_firstblock = new block_addr_t( m__io, this, m__root );
  m_maxfiles_per_block = m__io->read_u4le();
  m_nfiles = m__io->read_u4le();
}

uop_t::header_t::~header_t()
{
  delete m_firstblock;
}

uop_t::file_t::file_t( kaitai::kstream* p__io, uop_t::block_body_t* p__parent, uop_t* p__root )
    : kaitai::kstruct( p__io )
{
  m__parent = p__parent;
  m__root = p__root;
  f_data = false;
  _read();
}

void uop_t::file_t::_read()
{
  m_dataaddr = m__io->read_u8le();
  m_file_len = m__io->read_u4le();
  m_compressed_size = m__io->read_u4le();
  m_decompressed_size = m__io->read_u4le();
  m_filehash = m__io->read_u8le();
  m_adler32 = m__io->read_u4le();
  m_compression_type = static_cast<uop_t::compression_type_t>( m__io->read_u2le() );
}

uop_t::file_t::~file_t()
{
  if ( f_data && !n_data )
  {
    delete m__io__raw_data;
    delete m_data;
  }
}

uop_t::data_header_v5_t* uop_t::file_t::data()
{
  if ( f_data )
    return m_data;
  n_data = true;
  if ( dataaddr() > 0 )
  {
    n_data = false;
    std::streampos _pos = m__io->pos();
    m__io->seek( dataaddr() );
    m__raw_data = m__io->read_bytes( ( file_len() + compressed_size() ) );
    m__io__raw_data = new kaitai::kstream( m__raw_data );
    m_data = new data_header_v5_t( m__io__raw_data, this, m__root );
    m__io->seek( _pos );
  }
  f_data = true;
  return m_data;
}
