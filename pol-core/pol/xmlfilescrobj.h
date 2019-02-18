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
  virtual Bscript::BObject* step() override;

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
  virtual Bscript::BObject* step() override;

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
  ~BXMLfile() = default;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override { return sizeof( *this ) + _filename.capacity(); }
  virtual const char* typeOf() const override { return "XMLFile"; }
  virtual u8 typeOfInt() const override { return OTXMLFile; }
  virtual bool isTrue() const override;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
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
  ~BXmlNode();
  virtual Bscript::BObjectImp* copy() const override { return new BXmlNode( node ); }
  virtual std::string getStringRep() const override;

  virtual const char* typeOf() const override { return "XMLNode"; }
  virtual u8 typeOfInt() const override { return OTXMLNode; }
  virtual size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( TiXmlNode ); }
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLNodeIterator( node, pIterVal );
  }

  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
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

  virtual Bscript::BObjectImp* copy() const override { return new BXmlAttribute( node ); }

  virtual std::string getStringRep() const override { return "XMLAttributes"; }
  virtual const char* typeOf() const override { return "XMLAttributes"; }
  virtual u8 typeOfInt() const override { return OTXMLAttributes; }
  virtual size_t sizeEstimate() const override { return sizeof( *this ); }
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLAttributeIterator( node, pIterVal );
  }
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;

private:
  TiXmlElement* node;
};
}  // namespace Core
}  // namespace Pol

#endif
