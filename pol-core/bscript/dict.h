/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef BSCRIPT_DICT_H
#define BSCRIPT_DICT_H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <map>
#include <iosfwd>

namespace Pol {
  namespace Bscript {
	class BDictionary : public BObjectImp
	{
	public:
	  BDictionary();

	  static BObjectImp* unpack( std::istream& is );

	  void addMember( const char* name, BObjectRef val );
	  void addMember( const char* name, BObjectImp* imp );
	  void addMember( BObjectImp* key, BObjectImp* val );
	  size_t mapcount() const;

	  typedef std::map<BObject, BObjectRef> Contents;
	  const Contents& contents() const;

	protected:
	  BDictionary( std::istream& is, unsigned size, BObjectType type = OTDictionary );
	  BDictionary( const BDictionary&, BObjectType type = OTDictionary );

	  virtual BObjectImp* copy() const POL_OVERRIDE;
	  virtual std::string getStringRep() const POL_OVERRIDE;
	  virtual size_t sizeEstimate() const POL_OVERRIDE;
	  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
	  virtual const char* typeOf() const POL_OVERRIDE;
	  virtual u8 typeOfInt() const POL_OVERRIDE;

	  virtual ContIterator* createIterator( BObject* pIterVal ) POL_OVERRIDE;

	  virtual char packtype() const;
	  virtual const char* typetag() const;
	  virtual void FormatForStringRep( std::ostream& os, const BObject& bkeyobj, const BObjectRef& bvalref ) const;

	  virtual BObjectRef OperSubscript( const BObject& obj ) POL_OVERRIDE;
	  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
	  virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
	  virtual BObjectRef set_member( const char* membername, BObjectImp* value, bool copy ) POL_OVERRIDE;
	  virtual BObjectRef get_member( const char* membername ) POL_OVERRIDE;
	  virtual BObjectRef operDotPlus( const char* name ) POL_OVERRIDE;
	  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) POL_OVERRIDE;

	  friend class BDictionaryIterator;

	protected:
	  explicit BDictionary( BObjectType type );

	private:
	  Contents contents_;

	  // not implemented:
	  BDictionary& operator=( const BDictionary& );
	};
  }
}
#endif
