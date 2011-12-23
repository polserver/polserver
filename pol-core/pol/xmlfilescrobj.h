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


class BXMLNodeIterator : public ContIterator
{
public:
	BXMLNodeIterator( TiXmlDocument* file, BObject* pIter );
	BXMLNodeIterator( TiXmlNode* node, BObject* pIter );
	virtual BObject* step();
private:
	TiXmlNode* node;
	TiXmlDocument* _file;
	bool _init;
	BObjectRef m_IterVal;
	BLong* m_pIterVal;
};

class BXMLAttributeIterator : public ContIterator
{
public:
	BXMLAttributeIterator( TiXmlElement* node, BObject* pIter );
	virtual BObject* step();
private:
	TiXmlElement* node;
	TiXmlAttribute* nodeAttrib;
	BObjectRef m_IterVal;
	BLong* m_pIterVal;
};

class BXMLfile : public BObjectImp
{
public:
	BXMLfile();
	BXMLfile(std::string filename);
	~BXMLfile();
	virtual BObjectRef get_member( const char* membername );
	virtual BObjectRef get_member_id( const int id ); //id test
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
	virtual BObjectImp* copy() const;
	virtual std::string getStringRep() const;
	virtual size_t sizeEstimate() const { return sizeof(*this); }
	virtual const char* typeOf() const { return "XMLFile"; }
	virtual int typeOfInt() const {return OTXMLFile; }
	virtual bool isTrue() const;
	virtual BObjectRef OperSubscript( const BObject& obj );
	ContIterator* createIterator( BObject* pIterVal )
	{
		return new BXMLNodeIterator( &file, pIterVal );
	}

private:
	TiXmlDocument file;
	std::string _filename;
};



class BXmlNode : public BObjectImp
{
public:
	BXmlNode(TiXmlNode* _node) : BObjectImp( BObjectImp::OTXMLNode ), node(_node) {}

	virtual BObjectImp* copy() const
	{
		return new BXmlNode(node);
	}

	virtual std::string getStringRep() const
	{
		if (node->Type()==TiXmlNode::TINYXML_TEXT)
			return node->ToText()->Value();
		else if (node->Type()==TiXmlNode::TINYXML_DECLARATION)
		{
			TiXmlDeclaration* dec = node->ToDeclaration();
			OSTRINGSTREAM os;
			os << "v:" << dec->Version() << " e:" << dec->Encoding() << " s:" << dec->Standalone();
			return OSTRINGSTREAM_STR(os);
		}
		return node->Value();
	}
	
	virtual const char* typeOf() const { return "XMLNode"; }
	virtual int typeOfInt() const { return OTXMLNode; }

	virtual size_t sizeEstimate() const
	{
		return sizeof(*this);
	}
	ContIterator* createIterator( BObject* pIterVal )
	{
		return new BXMLNodeIterator( node, pIterVal );
	}

	virtual BObjectRef get_member( const char* membername );
	virtual BObjectRef get_member_id( const int id ); //id test
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
	virtual BObjectRef OperSubscript( const BObject& obj );

private:
	TiXmlNode* node;
};

class BXmlAttribute : public BObjectImp
{
public:
	BXmlAttribute(TiXmlNode* _node) : BObjectImp( BObjectImp::OTXMLAttributes ), node(_node->ToElement()) {}

	virtual BObjectImp* copy() const
	{
		return new BXmlAttribute(node);
	}

	virtual std::string getStringRep() const
	{
		return "XMLAttributes";
	}
	
	virtual const char* typeOf() const { return "XMLAttributes"; }
	virtual int typeOfInt() const { return OTXMLAttributes; }
	virtual size_t sizeEstimate() const { return sizeof(*this); }
	
	ContIterator* createIterator( BObject* pIterVal )
	{
		return new BXMLAttributeIterator( node, pIterVal );
	}
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
	virtual BObjectRef OperSubscript( const BObject& obj );

private:
	TiXmlElement* node;
};

#endif
