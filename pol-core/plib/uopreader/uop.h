#ifndef UOP_H_
#define UOP_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitai/kaitaistruct.h"

#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 7000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.7 or later is required"
#endif

class uop_t : public kaitai::kstruct
{
public:
  class block_addr_t;
  class block_body_t;
  class data_header_v5_t;
  class header_t;
  class file_t;

  enum compression_type_t
  {
    COMPRESSION_TYPE_NO_COMPRESSION = 0,
    COMPRESSION_TYPE_ZLIB = 1
  };

  uop_t( kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, uop_t* p__root = 0 );

private:
  void _read();

public:
  ~uop_t() override;

  class block_addr_t : public kaitai::kstruct
  {
  public:
    block_addr_t( kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, uop_t* p__root = 0 );

  private:
    void _read();

  public:
    ~block_addr_t() override;

  private:
    bool f_block_body;
    block_body_t* m_block_body;
    bool n_block_body;

  public:
    bool _is_null_block_body()
    {
      block_body();
      return n_block_body;
    };

  private:
  public:
    block_body_t* block_body();

  private:
    uint64_t m_blockaddr;
    uop_t* m__root;
    kaitai::kstruct* m__parent;

  public:
    uint64_t blockaddr() const { return m_blockaddr; }
    uop_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

  class block_body_t : public kaitai::kstruct
  {
  public:
    block_body_t( kaitai::kstream* p__io, uop_t::block_addr_t* p__parent = 0, uop_t* p__root = 0 );

  private:
    void _read();

  public:
    ~block_body_t() override;

  private:
    uint32_t m_nfiles;
    block_addr_t* m_next_addr;
    std::vector<file_t*>* m_files;
    uop_t* m__root;
    uop_t::block_addr_t* m__parent;

  public:
    uint32_t nfiles() const { return m_nfiles; }
    block_addr_t* next_addr() const { return m_next_addr; }
    std::vector<file_t*>* files() const { return m_files; }
    uop_t* _root() const { return m__root; }
    uop_t::block_addr_t* _parent() const { return m__parent; }
  };

  class data_header_v5_t : public kaitai::kstruct
  {
  public:
    data_header_v5_t( kaitai::kstream* p__io, uop_t::file_t* p__parent = 0, uop_t* p__root = 0 );

  private:
    void _read();

  public:
    ~data_header_v5_t() override;

  private:
    std::string m_bytes;
    std::string m_filebytes;
    uop_t* m__root;
    uop_t::file_t* m__parent;

  public:
    std::string bytes() const { return m_bytes; }
    std::string filebytes() const { return m_filebytes; }
    uop_t* _root() const { return m__root; }
    uop_t::file_t* _parent() const { return m__parent; }
  };

  class header_t : public kaitai::kstruct
  {
  public:
    header_t( kaitai::kstream* p__io, uop_t* p__parent = 0, uop_t* p__root = 0 );

  private:
    void _read();

  public:
    ~header_t() override;

  private:
    std::string m_magic;
    std::string m_version;
    std::string m_signature;
    block_addr_t* m_firstblock;
    uint32_t m_maxfiles_per_block;
    uint32_t m_nfiles;
    uop_t* m__root;
    uop_t* m__parent;

  public:
    std::string magic() const { return m_magic; }
    std::string version() const { return m_version; }
    std::string signature() const { return m_signature; }
    block_addr_t* firstblock() const { return m_firstblock; }
    uint32_t maxfiles_per_block() const { return m_maxfiles_per_block; }
    uint32_t nfiles() const { return m_nfiles; }
    uop_t* _root() const { return m__root; }
    uop_t* _parent() const { return m__parent; }
  };

  class file_t : public kaitai::kstruct
  {
  public:
    file_t( kaitai::kstream* p__io, uop_t::block_body_t* p__parent = 0, uop_t* p__root = 0 );

  private:
    void _read();

  public:
    ~file_t() override;

  private:
    bool f_data;
    data_header_v5_t* m_data;
    bool n_data;

  public:
    bool _is_null_data()
    {
      data();
      return n_data;
    };

  private:
  public:
    data_header_v5_t* data();

  private:
    uint64_t m_dataaddr;
    uint32_t m_file_len;
    uint32_t m_compressed_size;
    uint32_t m_decompressed_size;
    uint64_t m_filehash;
    uint32_t m_adler32;
    compression_type_t m_compression_type;
    uop_t* m__root;
    uop_t::block_body_t* m__parent;
    std::string m__raw_data;
    kaitai::kstream* m__io__raw_data;

  public:
    uint64_t dataaddr() const { return m_dataaddr; }
    uint32_t file_len() const { return m_file_len; }
    uint32_t compressed_size() const { return m_compressed_size; }
    uint32_t decompressed_size() const { return m_decompressed_size; }
    uint64_t filehash() const { return m_filehash; }
    uint32_t adler32() const { return m_adler32; }
    compression_type_t compression_type() const { return m_compression_type; }
    uop_t* _root() const { return m__root; }
    uop_t::block_body_t* _parent() const { return m__parent; }
    std::string _raw_data() const { return m__raw_data; }
    kaitai::kstream* _io__raw_data() const { return m__io__raw_data; }
  };

private:
  header_t* m_header;
  uop_t* m__root;
  kaitai::kstruct* m__parent;

public:
  header_t* header() const { return m_header; }
  uop_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // UOP_H_
