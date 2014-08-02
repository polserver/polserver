
#ifndef POL_PACKETDEFS_H
#define POL_PACKETDEFS_H

#include "packets.h"
#include "../../clib/rawtypes.h"

namespace Pol {
  namespace Mobile {
    class Character;
  }
  namespace Network {
    class Client;

    class PktSender
    {
    public:
      virtual void Send( Client* client ) =0;
    };

    class SendWorldItem : public PktSender
    {
    public:
      SendWorldItem( u32 serial, u16 graphic, u16 amount, u16 x, u16 y, s8 z, u8 facing, u16 color, u8 flags );
      virtual void Send( Client* client );
      void updateFlags( u8 flags );
    private:
      void buildF3();
      void build1A();
      u16 _p_oldlen;
      u32 _serial;
      u16 _graphic;
      u16 _amount;
      u16 _x;
      u16 _y;
      s8 _z;
      u8 _facing;
      u16 _color;
      u8 _flags;
      PktHelper::PacketOut<PktOut_1A> _p_old;
      PktHelper::PacketOut<PktOut_F3> _p;
    };

    class SendWorldMulti : public PktSender
    {
    public:
      SendWorldMulti( u32 serial_ext, u16 graphic, u16 x, u16 y, s8 z, u16 color );
      virtual void Send( Client* client );
    private:
      void buildF3();
      void build1A();
      u16 _p_oldlen;
      u32 _serial_ext;
      u16 _graphic;
      u16 _x;
      u16 _y;
      s8 _z;
      u16 _color;
      PktHelper::PacketOut<PktOut_1A> _p_old;
      PktHelper::PacketOut<PktOut_F3> _p;
    };

    class AddItemContainerMsg : public PktSender
    {
    public:
      AddItemContainerMsg( u32 serial_ext, u16 graphic, u16 amount, u16 x, u16 y, u8 slotindex, u32 containerserial_ext, u16 color );
      virtual void Send( Client* client );
    private:
      void buildLegacy();
      void build();
      u32 _serial_ext;
      u16 _graphic;
      u16 _amount;
      u16 _x;
      u16 _y;
      u8 _slotindex;
      u32 _containerserial_ext;
      u16 _color;
      PktHelper::PacketOut<PktOut_25> _p_old;
      PktHelper::PacketOut<PktOut_25> _p;
    };






  }
}
#endif
