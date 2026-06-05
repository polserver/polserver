#pragma once

#include "../bscript/bobject.h"
#include "../bscript/contiter.h"
#include "storage.h"

namespace Pol::Core
{
class StorageAreasImp final : public Bscript::BObjectImp
{
public:
  StorageAreasImp() : Bscript::BObjectImp( Bscript::BObjectImp::OTUnknown ) {}
  BObjectImp* copy() const override { return new StorageAreasImp(); }
  std::string getStringRep() const override { return "<StorageAreas>"; }
  size_t sizeEstimate() const override { return sizeof( *this ); }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
};

class StorageAreaImp final : public Bscript::BObjectImp
{
public:
  StorageAreaImp( StorageArea* area )
      : Bscript::BObjectImp( BObjectImp::OTStorageArea ), _area( area )
  {
  }
  BObjectImp* copy() const override { return new StorageAreaImp( _area ); }
  std::string getStringRep() const override { return _area->_name; }
  size_t sizeEstimate() const override { return sizeof( *this ); }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  const char* typeOf() const override { return "StorageArea"; }
  u8 typeOfInt() const override { return OTStorageArea; }
  StorageArea* area() const { return _area; }

private:
  StorageArea* _area;
};

class StorageAreasIterator final : public Bscript::ContIterator
{
public:
  StorageAreasIterator( Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  Bscript::BObject* m_pIterVal;
  std::string key;
};

class StorageAreaIterator final : public Bscript::ContIterator
{
public:
  StorageAreaIterator( StorageArea* area, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  Bscript::BObject* m_pIterVal;
  std::string key;
  StorageArea* _area;
};

Bscript::BObjectImp* CreateStorageAreasImp();
}  // namespace Pol::Core
