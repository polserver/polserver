/** @file
 *
 * @par History
 */


#ifndef UMAP_H
#define UMAP_H

#ifndef ITEM_H
#include "item/item.h"
#endif

#include "pktboth.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Items
{
class MapDesc;
}
namespace Core
{


struct PinPoint
{
  unsigned short x;
  unsigned short y;
};

class Map : public Items::Item
{
  typedef Items::Item base;
public:
  u16 gumpwidth;
  u16 gumpheight;
  bool editable;
  bool plotting;
  typedef std::vector<PinPoint> PinPoints;
  PinPoints pin_points;
  typedef PinPoints::iterator pin_points_itr;

  inline u16 get_xwest()
  {
    return xwest;
  };
  inline u16 get_xeast()
  {
    return xeast;
  };
  inline u16 get_ynorth()
  {
    return ynorth;
  };
  inline u16 get_ysouth()
  {
    return ysouth;
  };

  bool msgCoordsInBounds( PKTBI_56* msg );
  u16 gumpXtoWorldX( u16 gumpx );
  u16 gumpYtoWorldY( u16 gumpy );
  u16 worldXtoGumpX( u16 worldx );
  u16 worldYtoGumpY( u16 worldy );

  virtual Items::Item* clone( ) const POL_OVERRIDE; //dave 12-20
  virtual ~Map( );
  virtual size_t estimatedSize( ) const POL_OVERRIDE;
protected:
  Map( const Items::MapDesc& mapdesc );
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );

  virtual void builtin_on_use( Network::Client* client ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
  //virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE; //id test
  //virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value );
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  void printPinPoints( Clib::StreamWriter& sw ) const;
  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;


private:
  u16 xwest;
  u16 xeast;
  u16 ynorth;
  u16 ysouth;
  u16 facetid;

private: // not implemented
  Map( const Map& map );
  Map& operator=( const Map& map );
};
}
}
#endif

