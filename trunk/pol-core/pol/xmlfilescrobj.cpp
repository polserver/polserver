/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/strutil.h"
#include "../clib/stlutil.h"
#include "../clib/endian.h"

#include "xmlfilescrobj.h"

#include "../bscript/executor.h"
#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

BXMLfile::BXMLfile(): BObjectImp( OTXMLFile ),
_filename( "" )
{
}

BXMLfile::BXMLfile(string filename) : BObjectImp(OTXMLFile),
file( filename.c_str() ),
_filename( filename )
{
	if (!file.LoadFile())
		return;
}

BXMLfile::~BXMLfile()
{
}

BObjectRef BXMLfile::get_member_id( const int id )//id test
{
	return BObjectRef(UninitObject::create());
	//switch(id)
	//{

	//	default: return BObjectRef(UninitObject::create());
	//}
}
BObjectRef BXMLfile::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* BXMLfile::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* BXMLfile::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	return NULL;
	//switch(id)
	//{
	//	default:
	//		return NULL;
	//}
}

BObjectImp* BXMLfile::copy() const
{
	return new BXMLfile(_filename);
}

std::string BXMLfile::getStringRep() const
{
	return _filename;
}

bool BXMLfile::isTrue() const
{
	if (_filename=="")
		return false;
	return !file.Error();
}

BObjectRef BXMLfile::OperSubscript( const BObject& obj )
{
	if (obj->isa( OTString ))
	{
		const String* keystr = static_cast<const String*>(obj.impptr());
		TiXmlNode* node = file.FirstChild(keystr->value());
		if (node)
			return BObjectRef(new BXmlNode(node));
		else
			return BObjectRef(new BError( "Failed to find node" ));
	}
	else if (obj->isa( OTLong ))
	{
		BLong& keyint = (BLong&)obj.impref();
		TiXmlHandle handle(&file);
		TiXmlNode* node = handle.Child(keyint.value()-1).ToNode(); //keep escript 1based index and change it to 0based
		if (node)
			return BObjectRef(new BXmlNode(node));
		else
			return BObjectRef(new BError( "Failed to find node" ));
	}
	else
	{
		return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
	}
}

BObjectRef BXmlNode::get_member_id( const int id )//id test
{
	switch(id)
	{
	case MBR_ATTRIBUTES:
		if (node->ToElement())
			return BObjectRef(new BXmlAttribute(node));
		else
			return BObjectRef(new BError( "No attributes available." ));
		
	case MBR_TYPE:
		switch(node->Type())
		{
			case (TiXmlNode::TINYXML_COMMENT): return BObjectRef(new String("XMLComment"));
			case (TiXmlNode::TINYXML_DECLARATION): return BObjectRef(new String("XMLDeclaration"));
			case (TiXmlNode::TINYXML_DOCUMENT): return BObjectRef(new String("XMLDocument"));
			case (TiXmlNode::TINYXML_ELEMENT): return BObjectRef(new String("XMLElement"));
			case (TiXmlNode::TINYXML_TEXT): return BObjectRef(new String("XMLText"));
			case (TiXmlNode::TINYXML_TYPECOUNT): return BObjectRef(new String("XMLTypeCount"));
			case (TiXmlNode::TINYXML_UNKNOWN): return BObjectRef(new String("XMLUnknown"));
			default: return BObjectRef(new String("XMLUnknown"));
		}

	default: return BObjectRef(UninitObject::create());
	}

}
BObjectRef BXmlNode::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* BXmlNode::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* BXmlNode::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	switch(id)
	{
		case MTH_FIRSTCHILD:
		{
			if (ex.hasParams(1))
			{
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlNode* child = node->FirstChild(pstr->value());
					if (child)
						return new BXmlNode(child);
					else
						return new BError( "Failed to find node" );
				}
			}
			else
			{
				TiXmlNode* child = node->FirstChild();
				if (child)
					return new BXmlNode(child);
				else
					return new BError( "Failed to find node" );
			}

		}
		case MTH_NEXTSIBLING:
		{
			if (ex.hasParams(1))
			{
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlNode* sibling = node->NextSibling(pstr->value());
					if (sibling)
						return new BXmlNode(sibling);
					else
						return new BError( "Failed to find node" );
				}
			}
			else
			{
				TiXmlNode* sibling = node->NextSibling();
				if (sibling)
					return new BXmlNode(sibling);
				else
					return new BError( "Failed to find node" );
			}
		}
		default:
			return NULL;
	}
}

BObjectRef BXmlNode::OperSubscript( const BObject& obj )
{
	if (obj->isa( OTString ))
	{
		const String* keystr = static_cast<const String*>(obj.impptr());
		TiXmlNode* child = node->FirstChild(keystr->value());
		if (child)
			return BObjectRef(new BXmlNode(child));
		else
			return BObjectRef(new BError( "Failed to find node" ));
	}
	else if (obj->isa( OTLong ))
	{
		BLong& keyint = (BLong&)obj.impref();
		TiXmlHandle handle(node);
		TiXmlNode* child = handle.Child(keyint.value()-1).ToNode(); //keep escript 1based index and change it to 0based
		if (child)
			return BObjectRef(new BXmlNode(child));
		else
			return BObjectRef(new BError( "Failed to find node" ));
	}
	else
	{
		return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
	}
}

BObjectImp* BXmlAttribute::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* BXmlAttribute::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	if (!node)
		return NULL;
	switch(id)
	{
		case MTH_PROPNAMES:
		{
			auto_ptr<ObjArray> arr (new ObjArray);
			TiXmlAttribute* attrib=node->FirstAttribute();
			while(attrib)
			{
				arr->addElement(new String(attrib->Name()));
				attrib=attrib->Next();
			}
			return arr.release();
		}
		default:
			return NULL;
	}
}

BObjectRef BXmlAttribute::OperSubscript( const BObject& obj )
{
	if (obj->isa( OTString ))
	{
		const String* keystr = static_cast<const String*>(obj.impptr());
		const string* attrib = node->Attribute(keystr->value());
		if (attrib)
			return BObjectRef( new String(attrib->c_str()) );
		else
			return BObjectRef(new BError( "Failed to find attribute" ));
	}
	else
	{
		return BObjectRef( new BError( "xml attribute can only be accessed by name" ) );
	}
}

BXMLNodeIterator::BXMLNodeIterator( TiXmlDocument* file, BObject* pIter ) : //root elements
node(NULL),
_file(file),
_init(false),
m_IterVal( pIter ),
m_pIterVal( new BLong(0) )
{
	m_IterVal.get()->setimp( m_pIterVal );
}
BXMLNodeIterator::BXMLNodeIterator( TiXmlNode* _node, BObject* pIter ) : // child elements
node(_node),
_file(NULL),
_init(false),
m_IterVal( pIter ),
m_pIterVal( new BLong(0) )
{
	m_IterVal.get()->setimp( m_pIterVal );
}

BObject* BXMLNodeIterator::step()
{
	m_pIterVal->increment();
	if (node == NULL) //root elements (iter over BXmlFile)
		node = _file->FirstChild();
	else if (_file == NULL) //child elements (iter over BXmlNode)
	{
		if (!_init)
		{
			_init = true;
			node = node->FirstChild();
		}
		else
			node = node->NextSiblingElement();
	}
	else //root elements (iter over BXmlFile)
		node = node->NextSibling();
	if (!node)
		return NULL;
	
	return new BObject(new BXmlNode(node));
}

BXMLAttributeIterator::BXMLAttributeIterator( TiXmlElement* _node, BObject* pIter ) :
node(_node),
nodeAttrib(NULL),
m_IterVal( pIter ),
m_pIterVal( new BLong(0) )
{
	m_IterVal.get()->setimp( m_pIterVal );
}

BObject* BXMLAttributeIterator::step()
{
	m_pIterVal->increment();
	if (!node)
		return NULL;
	if (nodeAttrib==NULL)
		nodeAttrib=node->FirstAttribute();
	else
		nodeAttrib=nodeAttrib->Next();
	if (nodeAttrib)
	{
		auto_ptr<BStruct> details (new BStruct());
		details->addMember(nodeAttrib->Name(), new String(nodeAttrib->Value()));
		return new BObject(details.release());
	}
	return NULL;
}