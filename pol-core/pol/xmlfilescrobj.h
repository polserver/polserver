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
#include "../clib/compilerspecifics.h"
#include "../clib/rawtypes.h"
#include "../clib/unicode.h"

namespace Pol {
namespace Bscript {
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{
class BXMLNodeIterator : public Bscript::ContIterator
{
public:
  BXMLNodeIterator( TiXmlDocument* file, Bscript::BObject* pIter );
  BXMLNodeIterator( TiXmlNode* node, Bscript::BObject* pIter );
  virtual Bscript::BObject* step() POL_OVERRIDE;

private:
  TiXmlNode* node;
  TiXmlDocument* _file;
  bool _init;
  Bscript::BObjectRef m_IterVal;
  Bscript::BLong* m_pIterVal;
};

class BXMLAttributeIterator : public Bscript::ContIterator
{
public:
  BXMLAttributeIterator( TiXmlElement* node, Bscript::BObject* pIter );
  virtual Bscript::BObject* step() POL_OVERRIDE;

private:
  TiXmlElement* node;
  TiXmlAttribute* nodeAttrib;
  Bscript::BObjectRef m_IterVal;
  Bscript::BLong* m_pIterVal;
};

class BXMLfile : public Bscript::BObjectImp
{
public:
  BXMLfile();
  BXMLfile( std::string filename );
  ~BXMLfile();
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;
  virtual UnicodeString getStringRep() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE
  {
    return sizeof( *this ) + _filename.capacity();
  }
  virtual const char* typeOf() const POL_OVERRIDE { return "XMLFile"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTXMLFile; }
  virtual bool isTrue() const POL_OVERRIDE;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) POL_OVERRIDE;
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) POL_OVERRIDE
  {
    return new BXMLNodeIterator( &file, pIterVal );
  }

private:
  TiXmlDocument file;
  std::string _filename;
};


class BXmlNode : public Bscript::BObjectImp
{
public:
  BXmlNode( TiXmlNode* _node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ), node( _node )
  {
  }

  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE { return new BXmlNode( node->Clone() ); }
  virtual UnicodeString getStringRep() const POL_OVERRIDE;

  virtual const char* typeOf() const POL_OVERRIDE { return "XMLNode"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTXMLNode; }
  virtual size_t sizeEstimate() const POL_OVERRIDE { return sizeof( *this ) + sizeof( TiXmlNode ); }
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) POL_OVERRIDE
  {
    return new BXMLNodeIterator( node, pIterVal );
  }

  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) POL_OVERRIDE;
  TiXmlNode* getNode() const { return node; }
private:
  TiXmlNode* node;
};

class BXmlAttribute : public Bscript::BObjectImp
{
public:
  BXmlAttribute( TiXmlNode* _node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLAttributes ), node( _node->ToElement() )
  {
  }

  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE
  {
    return new BXmlAttribute( node->Clone() );
  }

  virtual UnicodeString getStringRep() const POL_OVERRIDE { return "XMLAttributes"; }
  virtual const char* typeOf() const POL_OVERRIDE { return "XMLAttributes"; }
  virtual u8 typeOfInt() const POL_OVERRIDE { return OTXMLAttributes; }
  virtual size_t sizeEstimate() const POL_OVERRIDE { return sizeof( *this ); }
  virtual Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) POL_OVERRIDE
  {
    return new BXMLAttributeIterator( node, pIterVal );
  }
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) POL_OVERRIDE;

private:
  TiXmlElement* node;
};
}
}

#endif
