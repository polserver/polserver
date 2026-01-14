/** @file
 *
 * @par History
 */


#ifndef XMLSCROBJ_H
#define XMLSCROBJ_H

#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include <string>
#include <tinyxml/tinyxml.h>

#include "../bscript/contiter.h"
#include "../clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{
class BXMLNodeIterator final : public Bscript::ContIterator
{
public:
  BXMLNodeIterator( TiXmlDocument* file, Bscript::BObject* pIter );
  BXMLNodeIterator( TiXmlNode* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  TiXmlNode* node;
  TiXmlDocument* _file;
  bool _init;
  Bscript::BObjectRef m_IterVal;
  Bscript::BLong* m_pIterVal;
};

class BXMLAttributeIterator final : public Bscript::ContIterator
{
public:
  BXMLAttributeIterator( TiXmlElement* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  TiXmlElement* node;
  TiXmlAttribute* nodeAttrib;
  Bscript::BObjectRef m_IterVal;
  Bscript::BLong* m_pIterVal;
};

class BXMLfile final : public Bscript::BObjectImp
{
public:
  BXMLfile();
  BXMLfile( std::string filename );
  ~BXMLfile() override = default;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
  Bscript::BObjectImp* copy() const override;
  std::string getStringRep() const override;
  size_t sizeEstimate() const override { return sizeof( *this ) + _filename.capacity(); }
  const char* typeOf() const override { return "XMLFile"; }
  u8 typeOfInt() const override { return OTXMLFile; }
  bool isTrue() const override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLNodeIterator( &file, pIterVal );
  }

private:
  TiXmlDocument file;
  std::string _filename;
};


class BXmlNode final : public Bscript::BObjectImp
{
public:
  BXmlNode( TiXmlNode* _node, bool cloned = false )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ), node( _node ), _cloned( cloned )
  {
  }
  ~BXmlNode() override;
  Bscript::BObjectImp* copy() const override { return new BXmlNode( node ); }
  std::string getStringRep() const override;

  const char* typeOf() const override { return "XMLNode"; }
  u8 typeOfInt() const override { return OTXMLNode; }
  size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( TiXmlNode ); }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLNodeIterator( node, pIterVal );
  }

  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  TiXmlNode* getNode() const { return node; }

private:
  TiXmlNode* node;
  bool _cloned;
};

class BXmlAttribute final : public Bscript::BObjectImp
{
public:
  BXmlAttribute( TiXmlNode* _node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLAttributes ), node( _node->ToElement() )
  {
  }

  Bscript::BObjectImp* copy() const override { return new BXmlAttribute( node ); }

  std::string getStringRep() const override { return "XMLAttributes"; }
  const char* typeOf() const override { return "XMLAttributes"; }
  u8 typeOfInt() const override { return OTXMLAttributes; }
  size_t sizeEstimate() const override { return sizeof( *this ); }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLAttributeIterator( node, pIterVal );
  }
  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;

private:
  TiXmlElement* node;
};
}  // namespace Core
}  // namespace Pol

#endif
