meta:
  id: uop
  endian: le
  file-extension: uop
  ks-opaque-types: true

params:
  - id: mapformat
    type: bool

seq:
  - id: header
    type: header
  
types:
  header:
    seq:
      - id: magic
        contents: ["MYP",0]
      - id: version
        size: 4
      - id: signature
        size: 4
      - id: firstblock
        type: block_addr
      - id: maxfiles_per_block
        type: u4
      - id: nfiles
        type: u4
  
  block_body:
    seq:
      - id: nfiles
        type: u4
      - id: next_addr
        type: block_addr
      - id: files
        type: file
        repeat: expr
        repeat-expr: nfiles
        
  block_addr:
    seq: 
      - id: blockaddr
        type: u8
    instances:
      block_body:
        if: blockaddr > 0
        pos: blockaddr
        type: block_body
  

  data_header_v5:
    seq:
      - id: bytes
        size: _parent.file_len
      - id: map_struct
        size: _parent.compressed_size
        type: map_struct
        if: _root.mapformat
      - id: filebytes
        size: _parent.compressed_size
        if: not _root.mapformat
     
      
  file:
    seq:
      - id: dataaddr
        type: u8
      - id: file_len
        type: u4
      - id: compressed_size
        type: u4
      - id: decompressed_size
        type: u4
      - id: filehash
        type: u8
      - id: adler32
        type: u4
      - id: compression_type
        enum: compression_type
        type: u2
    instances:
      data:
        if: dataaddr > 0
        pos: dataaddr
        type: data_header_v5
        size: file_len+compressed_size
        
enums:
  compression_type:
    0: no_compression
    1: zlib
