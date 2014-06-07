/*
History
=======


Notes
=======

*/

#ifndef XMLSCROBJ_H
#define XMLSCROBJ_H

#include "../../lib/tinyxml/tinyxml.h"
#include "../bscript/bobject.h"
#include "../bscript/contiter.h"

namespace Pol {
  namespace Core {
	class BXMLNodeIterator : public Bscript::ContIterator
	{
	public:
      BXMLNodeIterator( TiXmlDocument* file, Bscript::BObject* pIter );
      BXMLNodeIterator( TiXmlNode* node, Bscript::BObject* pIter );
      virtual Bscript::BObject* step( );
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
      virtual Bscript::BObject* step( );
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
      virtual Bscript::BObjectRef get_member( const char* membername );
      virtual Bscript::BObjectRef get_member_id( const int id ); //id test
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectImp* copy() const;
	  virtual std::string getStringRep() const;
      virtual size_t sizeEstimate( ) const { return sizeof( *this ) + _filename.capacity(); }
	  virtual const char* typeOf() const { return "XMLFile"; }
	  virtual int typeOfInt() const { return OTXMLFile; }
	  virtual bool isTrue() const;
      virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );
      Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal )
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
	  BXmlNode( TiXmlNode* _node ) : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ), node( _node ) {}

	  virtual Bscript::BObjectImp* copy() const
	  {
		return new BXmlNode( node->Clone() );
	  }

	  virtual std::string getStringRep() const
	  {
		if ( node->Type() == TiXmlNode::TINYXML_TEXT )
		  return node->ToText()->Value();
		else if ( node->Type() == TiXmlNode::TINYXML_DECLARATION )
		{
		  TiXmlDeclaration* dec = node->ToDeclaration();
		  OSTRINGSTREAM os;
		  os << "v:" << dec->Version() << " e:" << dec->Encoding() << " s:" << dec->Standalone();
		  return OSTRINGSTREAM_STR( os );
		}
		return node->Value();
	  }

	  virtual const char* typeOf() const { return "XMLNode"; }
	  virtual int typeOfInt() const { return OTXMLNode; }

	  virtual size_t sizeEstimate() const
	  {
        return sizeof( *this ) + sizeof( TiXmlNode );
	  }
      Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal )
	  {
		return new BXMLNodeIterator( node, pIterVal );
	  }

      virtual Bscript::BObjectRef get_member( const char* membername );
      virtual Bscript::BObjectRef get_member_id( const int id ); //id test
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
      virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );
	  TiXmlNode* getNode() const { return node; }

	private:
	  TiXmlNode* node;
	};

	class BXmlAttribute : public Bscript::BObjectImp
	{
	public:
	  BXmlAttribute( TiXmlNode* _node ) : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLAttributes ), node( _node->ToElement() ) {}

	  virtual Bscript::BObjectImp* copy() const
	  {
		return new BXmlAttribute( node->Clone() );
	  }

	  virtual std::string getStringRep() const
	  {
		return "XMLAttributes";
	  }

	  virtual const char* typeOf() const { return "XMLAttributes"; }
	  virtual int typeOfInt() const { return OTXMLAttributes; }
	  virtual size_t sizeEstimate() const { return sizeof( *this ); }

      Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal )
	  {
		return new BXMLAttributeIterator( node, pIterVal );
	  }
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
      virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );

	private:
	  TiXmlElement* node;
	};
  }
}

#endif
