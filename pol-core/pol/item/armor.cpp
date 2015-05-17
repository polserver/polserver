/*
History
=======
 2003/07/10 Dave: armordesc expects string zone names

Notes
=======

*/

#include "armor.h"
#include "item.h"
#include "armrtmpl.h"
#include "itemdesc.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"

#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/streamsaver.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"

#include "../tiles.h"
#include "../clidata.h"
#include "../layers.h"
#include "../ufunc.h"

#include <climits>
#include <stdexcept>

namespace Pol {
  namespace Mobile {
    const char* zone_to_zone_name( unsigned short zone );
    unsigned short layer_to_zone( unsigned short layer );
    unsigned short zone_name_to_zone( const char *zname );
  }
  namespace Items {

	
	

	ArmorDesc::ArmorDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg ) :
	  EquipDesc( objtype, elem, ARMORDESC, pkg ),
	  ar( elem.remove_ushort( "AR", 0 ) ),
	  zones(),
	  on_hit_script( elem.remove_string( "ONHITSCRIPT", "" ), pkg, "scripts/items/" )
	{
	  std::string coverage;
	  while ( elem.remove_prop( "COVERAGE", &coverage ) )
	  {
		try
		{
		  zones.insert( Mobile::zone_name_to_zone( coverage.c_str() ) );
		}
        catch (std::runtime_error&)
		{
			fmt::Writer tmp;
			tmp.Format( "Error in Objtype 0x{:X}" ) << objtype;
			if ( pkg == NULL )
				tmp << "config/itemdesc.cfg\n";
			else
				tmp << pkg->dir( ) << "itemdesc.cfg\n";

			ERROR_PRINT << tmp.str();
			throw;
		}
	  }

	  if ( zones.empty() )
	  {
		// No 'COVERAGE' entries existed.
		// default coverage based on object type/layer
		unsigned short layer = Plib::systemstate.tile[graphic].layer;
		// special case for shields - they effectively have no coverage.
		if ( layer != Core::LAYER_HAND1 && layer != Core::LAYER_HAND2 )
		{
		  try
		  {
			zones.insert( Mobile::layer_to_zone( layer ) );
		  }
          catch (std::runtime_error&)
		  {
			fmt::Writer tmp;
			tmp.Format( "Error in Objtype 0x{:X}" ) << objtype;
			if ( pkg == NULL )
				tmp << "config/itemdesc.cfg\n";
			else
				tmp << pkg->dir( ) << "itemdesc.cfg\n";

			ERROR_PRINT << tmp.str();
			throw;
		  }
		}
	  }
	}

	void ArmorDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
	{
	  base::PopulateStruct( descriptor );
	  descriptor->addMember( "OnHitScript", new Bscript::String( on_hit_script.relativename( pkg ) ) );
	  descriptor->addMember( "AR", new Bscript::BLong( ar ) );

	  std::unique_ptr<Bscript::ObjArray> arr_zones( new Bscript::ObjArray( ) );
	  std::set<unsigned short>::const_iterator itr;
	  for ( itr = zones.begin(); itr != zones.end(); ++itr )
		arr_zones->addElement( new Bscript::String( Mobile::zone_to_zone_name( *itr ) ) );


	  if ( arr_zones->ref_arr.size() > 0 )
		descriptor->addMember( "Coverage", arr_zones.release() );
	}

    size_t ArmorDesc::estimatedSize() const
    {
      size_t size = base::estimatedSize()
        + sizeof(unsigned short) /*ar*/
        + on_hit_script.estimatedSize();
      size += 3 * sizeof(void*)+zones.size( ) * ( sizeof(unsigned short)+3 * sizeof( void* ) );
      return size;
    }

	UArmor::UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor ) :
	  Equipment( descriptor, CLASS_ARMOR ),
	  tmpl( permanent_descriptor ),
	  onhitscript_( descriptor.on_hit_script )
	{}

	unsigned short UArmor::ar() const
	{
      short ar_mod = this->ar_mod();
	  int ar = tmpl->ar * hp_ / maxhp();
	  if ( ar_mod != 0 )
	  {
		ar += ar_mod;
	  }

	  if ( ar < 0 )
		return 0;
	  else if ( ar <= USHRT_MAX )
		return static_cast<unsigned short>( ar );
	  else
		return USHRT_MAX;
	}

	bool UArmor::covers( unsigned short layer ) const
	{
      passert(tmpl != NULL);
	  return tmpl->zones.find( layer ) != tmpl->zones.end();
	}

	Item* UArmor::clone() const
	{
	  UArmor* armor = static_cast<UArmor*>( base::clone() );
	  armor->ar_mod( this->ar_mod() );
	  armor->onhitscript_ = onhitscript_;
	  armor->tmpl = tmpl;
	  return armor;
	}

	void UArmor::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );
	  if ( has_ar_mod() )
		sw() << "\tAR_mod\t" << ar_mod() << pf_endl;
	  if ( tmpl != NULL && onhitscript_ != tmpl->on_hit_script )
		sw() << "\tOnHitScript\t" << onhitscript_.relativename( tmpl->pkg ) << pf_endl;
	}

	void UArmor::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );
	  this->ar_mod(static_cast<s16>( elem.remove_int( "AR_MOD", 0 ) ) );
	  set_onhitscript( elem.remove_string( "ONHITSCRIPT", "" ) );
	}

    void UArmor::set_onhitscript(const std::string& scriptname)
	{
	  if ( scriptname.empty() )
	  {
		onhitscript_.clear();
	  }
	  else
	  {
		onhitscript_.config( scriptname,
							 itemdesc().pkg,
							 "scripts/items/",
							 true );
	  }
	}
    std::set<unsigned short> UArmor::tmplzones( ) 
    { 
      passert(tmpl != NULL);
      return tmpl->zones;
    }

    size_t UArmor::estimatedSize() const
    {
      size_t size = base::estimatedSize()
        + sizeof(const ArmorDesc*) /*tmpl*/
        + onhitscript_.estimatedSize();
      return size;
    }
  }
}