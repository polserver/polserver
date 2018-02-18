meta:
  id: tiledata
  file-extension: mul
  endian: le
  encoding: ascii

params:
  - id: hsaformat
    type: bool

seq:
  - id: landtiles
    type: land(_index)
    repeat: expr
    repeat-expr: 0x4000
  - id: items
    type: art(_index)
    repeat: eos
    
types:
  land:
    params:
      - id: id
        type: u2
    seq:
      - id: header
        type: u4
        if: id % 32== 0
      - id: flags
        type: u4
        enum: flags
      - id: unkown
        type: u4
        if: _parent.hsaformat
      - id: texid
        type: u2
      - id: name
        type: str
        size: 20
        
  art:
    params:
      - id: id
        type: u2
    seq:
     - id: header
       type: u4
       if: id % 32 == 0
     - id: flags
       type: u4
       enum: flags
     - id: unk1
       type: u4
       if: _parent.hsaformat
     - id: weight
       type: u1
     - id: layer
       type: u1
     - id: miscdata
       type: u2
     - id: unk2
       type: u1
     - id: quantity
       type: u1
     - id: anim
       type: u2
     - id: unk3
       type: u1
     - id: hue
       type: u1
     - id: stackoffset
       type: u1
     - id: value
       type: u1
     - id: height
       type: u1
     - id: name
       type: str
       size: 20
       
enums:
  flags:
    0x0 : none
    0x1 : background
    0x2 : weapon
    0x4 : transparent
    0x8 : translucent
    0x10: wall
    0x20: damaging
    0x40: impassable
    0x80: wet
    0x100: unknown1
    0x200: surface
    0x400: bridge
    0x800: generic
    0x1000: window
    0x2000: noshoot
    0x4000: articlea
    0x8000: articlean
    0x10000: internal
    0x20000: foliage
    0x40000: partialhue
    0x80000: unknown2
    0x100000: map
    0x200000: container
    0x400000: wearable
    0x800000: lightsource
    0x1000000: animation
    0x2000000: hoverover
    0x4000000: unknown3
    0x8000000: armor
    0x10000000: roof
    0x20000000: door
    0x40000000: stairback
    0x80000000: stairright


  
