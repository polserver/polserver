/*
History
=======
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of realm and saveonexit_
2009/09/14 MuadDib:   UObject::setgraphic added error printing.
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <atomic>

#include "../clib/cfgelem.h"
#include "../clib/endian.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/streamsaver.h"

#include "../plib/realm.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "objtype.h"
#include "polclass.h"
#include "polcfg.h"
#include "realms.h"
#include "reftypes.h"
#include "tooltips.h"
#include "ufunc.h"
#include "uobjcnt.h"
#include "uofile.h"
#include "zone.h"

#include "uobject.h"

#include "objecthash.h"
namespace Pol {
  namespace Core {
	std::set<UObject*> unreaped_orphan_instances;
	ofstream orphans_txt( "orphans.txt", ios::out | ios::trunc );

	int display_orphan( UObject* o )
	{
	  bool tmp = false;
      Clib::FMTStreamWriter sw;
      Clib::OFStreamWriter sw_orphan( &orphans_txt );
      sw() << o->name() << ", " << o->ref_counted_count() << '\n';
	  if ( tmp ) o->printOn( sw );
	  o->printOnDebug( sw_orphan );
      INFO_PRINT << sw().c_str();
	  //ref_ptr<UObject>::display_referers( o->as_ref_counted() );

	  return 0;
	}
	void display_unreaped_orphan_instances()
	{
	  //    orphans_txt.open( "orphans.txt", ios::out|ios::trunc );

      for ( auto &obj : unreaped_orphan_instances )
      {
        display_orphan( obj );
      }
	  //for( std::set<UObject*>::iterator itr = unreaped_orphan_instances.begin();
	}

	std::atomic<unsigned int> UObject::dirty_writes;
	std::atomic<unsigned int> UObject::clean_writes;

	UObject::UObject( u32 objtype, UOBJ_CLASS i_uobj_class ) :
	  serial( 0 ),
	  serial_ext( 0 ),
	  objtype_( objtype ),
	  graphic( static_cast<u16>( objtype ) ),
	  color( 0 ),
	  x( 0 ),
	  y( 0 ),
	  z( 0 ),
	  facing( FACING_N ),
	  realm( NULL ),
	  saveonexit_( true ),
	  uobj_class_( static_cast<const u8>( i_uobj_class ) ),
	  dirty_( true ),
	  _rev( 0 ),
	  name_( "" )
	{
	  graphic = Items::getgraphic( objtype );

	  height = tileheight( graphic );
	  ++uobject_count;
	}

	UObject::~UObject()
	{
	  if ( ref_counted::count() != 0 )
	  {
        POLLOG_INFO << "Ouch! UObject::~UObject() with count()==" << ref_counted::count() << "\n";
	  }
	  passert( ref_counted::count() == 0 );
	  if ( serial == 0 )
	  {
		--unreaped_orphans;
	  }
	  --uobject_count;
	}

    size_t UObject::estimatedSize() const
    {
      size_t size = sizeof(UObject) + name_.capacity() + proplist_.estimatedSize();
      size += dynmap.size() * (sizeof( unsigned short ) + sizeof( boost::any ) * 2 + ( sizeof(void*)* 3 + 1 ) / 2);
      return size;
    }

	//
	//    Every UObject is registered with the objecthash after creation.
	//    (This can't happen in the ctor since the object isn't fully created yet)
	//
	//    Scripts may still reference any object, so rather than outright delete,
	//    we set its serial to 0 (marking it "orphan", though "zombie" would probably be a better term). 
	//    Later, when all _other_ references to the object have been deleted, 
	//    objecthash.Reap() will remove its reference to this object, deleting it.
	//
	void UObject::destroy()
	{
	  if ( serial != 0 )
	  {
		if ( ref_counted::count() < 1 )
		{
		  POLLOG_INFO << "Ouch! UObject::destroy() with count()==" << ref_counted::count() << "\n";
		}

		set_dirty(); // we will have to write a 'object deleted' directive once

		serial = 0; // used to set serial_ext to 0.  This way, if debugging, one can find out the old serial
		passert( ref_counted::count() >= 1 );

		++unreaped_orphans;
	  }
	}

	bool UObject::dirty() const
	{
	  return dirty_;
	}

	void UObject::clear_dirty() const
	{
	  if ( dirty_ )
		++dirty_writes;
	  else
		++clean_writes;
	  dirty_ = false;
	}

	bool UObject::getprop( const std::string& propname, std::string& propval ) const
	{
	  return proplist_.getprop( propname, propval );
	}

	void UObject::setprop( const std::string& propname, const std::string& propvalue )
	{
	  if ( propname[0] != '#' )
		set_dirty();
	  proplist_.setprop( propname, propvalue ); // VOID_RETURN
	}

	void UObject::eraseprop( const std::string& propname )
	{
	  if ( propname[0] != '#' )
		set_dirty();
	  proplist_.eraseprop( propname ); // VOID_RETURN
	}

	void UObject::copyprops( const UObject& from )
	{
	  set_dirty();
	  proplist_.copyprops( from.proplist_ );
	}

	void UObject::copyprops( const PropertyList& proplist )
	{
	  set_dirty();
	  proplist_.copyprops( proplist );
	}

	void UObject::getpropnames( std::vector< std::string >& propnames ) const
	{
	  proplist_.getpropnames( propnames );

	}

	const PropertyList& UObject::getprops() const
	{
	  return proplist_;
	}

	string UObject::name() const
	{
	  return name_;
	}

	string UObject::description() const
	{
	  return name_;
	}

	void UObject::setname( const string& newname )
	{
	  set_dirty();
	  increv();
	  send_object_cache_to_inrange( this );
	  name_ = newname;
	}

	UObject* UObject::owner()
	{
	  return NULL;
	}

	const UObject* UObject::owner() const
	{
	  return NULL;
	}

	UObject* UObject::self_as_owner()
	{
	  return this;
	}

	const UObject* UObject::self_as_owner() const
	{
	  return this;
	}

	UObject* UObject::toplevel_owner()
	{
	  return this;
	}

	const UObject* UObject::toplevel_owner() const
	{
	  return this;
	}


    void UObject::printProperties( Clib::StreamWriter& sw ) const
	{
	  using namespace fmt;

	  if ( !name_.empty() )
		sw() << "\tName\t" << name_ << pf_endl;

	  sw() << "\tSerial\t0x" << hex( serial ) << pf_endl;
	  sw() << "\tObjType\t0x" << hex( objtype_ ) << pf_endl;
	  sw() << "\tGraphic\t0x" << hex( graphic ) << pf_endl;

	  if ( color != 0 )
		sw() << "\tColor\t0x" << hex( color ) << pf_endl;

	  sw() << "\tX\t" << x << pf_endl;
	  sw() << "\tY\t" << y << pf_endl;
	  sw() << "\tZ\t" << (int)z << pf_endl;

	  if ( facing )
		sw() << "\tFacing\t" << static_cast<int>( facing ) << pf_endl;

	  sw() << "\tRevision\t" << rev() << pf_endl;
	  if ( realm == NULL )
		sw() << "\tRealm\tbritannia" << pf_endl;
	  else
		sw() << "\tRealm\t" << realm->name() << pf_endl;

	  proplist_.printProperties( sw );
	}

    void UObject::printDebugProperties( Clib::StreamWriter& sw ) const
	{
	  sw() << "# uobj_class: " << (int)uobj_class_ << pf_endl;
	}

    void UObject::readProperties( Clib::ConfigElem& elem )
	{
	  name_ = elem.remove_string( "NAME", "" );

	  // serial, objtype extracted by caller
	  graphic = elem.remove_ushort( "GRAPHIC", static_cast<u16>( objtype_ ) );
	  if ( graphic > ( config.max_tile_id ) )
		graphic = GRAPHIC_NODRAW;

	  height = tileheight( graphic );

	  color = elem.remove_ushort( "COLOR", 0 );


	  string realmstr = elem.remove_string( "Realm", "britannia" );
	  realm = find_realm( realmstr );
	  if ( !realm )
	  {
        ERROR_PRINT.Format( "{} '{}' (0x{:X}): has an invalid realm property '{}'.\n" ) << classname() << name() << serial << realmstr;
		throw runtime_error( "Data integrity error" );
	  }
	  x = elem.remove_ushort( "X" );
	  y = elem.remove_ushort( "Y" );
	  z = static_cast<s8>( elem.remove_int( "Z" ) );
	  if ( !realm->valid( x, y, z ) )
	  {
		x = static_cast<u16>( realm->width() ) - 1;
		y = static_cast<u16>( realm->height() ) - 1;
		z = 0;
	  }

	  unsigned short tmp = elem.remove_ushort( "FACING", 0 );
	  setfacing( static_cast<unsigned char>( tmp ) );

	  _rev = elem.remove_ulong( "Revision", 0 );


	  proplist_.readProperties( elem );

	}

    void UObject::printSelfOn( Clib::StreamWriter& sw ) const
	{
	  printOn( sw );
	}

    void UObject::printOn( Clib::StreamWriter& sw ) const
	{
	  sw() << classname() << pf_endl;
	  sw() << "{" << pf_endl;
	  printProperties( sw );
	  sw() << "}" << pf_endl;
	  sw() << pf_endl;
	  //sw.flush();
	}

    void UObject::printOnDebug( Clib::StreamWriter& sw ) const
	{
	  sw() << classname() << pf_endl;
	  sw() << "{" << pf_endl;
	  printProperties( sw );
	  printDebugProperties( sw );
	  sw() << "}" << pf_endl;
	  sw() << pf_endl;
	  //sw.flush();
	}

    Clib::StreamWriter& operator << ( Clib::StreamWriter& writer, const UObject& obj )
	{
	  obj.printOn( writer );
	  return writer;
	}

	bool UObject::setgraphic( u16 newgraphic )
	{
      ERROR_PRINT.Format( "UOBject::SetGraphic used, object class does not have a graphic member! Object Serial: 0x{:X}\n" ) << serial;
	  return false;
	}

	bool UObject::setcolor( u16 newcolor )
	{
	  set_dirty();

	  if ( color != newcolor )
	  {
		color = newcolor;
		on_color_changed();
	  }

	  return true;
	}

	void UObject::on_color_changed()
	{}

	void UObject::on_facing_changed()
	{}

	bool UObject::saveonexit() const
	{
	  return saveonexit_;
	}

	void UObject::saveonexit( bool newvalue )
	{
	  saveonexit_ = newvalue;
	}

	const char* UObject::target_tag() const
	{
	  return "object";
	}

  }
}