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
#include "../bscript/execmodl.h"
#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

#include "../plib/pkg.h"
#include "module/filemod.h"

BXMLfile::BXMLfile(): BObjectImp( OTXMLFile ),
file(),
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
	switch(id)
	{
		case MTH_SETDECLARATION:
			{
				if (!ex.hasParams(3))
					return new BError( "Not enough parameters" );
				const String* version;
				const String* encoding;
				const String* standalone;
				if (ex.getStringParam( 0, version ) &&
					ex.getStringParam( 1, encoding ) &&
					ex.getStringParam( 2, standalone ))
				{
					TiXmlDeclaration* decl = new TiXmlDeclaration( version->value(), encoding->value(), standalone->value() );  
					if (!file.NoChildren()) // in case its not the first method used
					{
						if (file.FirstChild()->Type()==TiXmlNode::TINYXML_DECLARATION)
							file.RemoveChild(file.FirstChild()); // remove old declaration
						if (!file.NoChildren())
							file.InsertBeforeChild(file.FirstChild(),*decl);
						else
							file.LinkEndChild( decl );
					}
					else
						file.LinkEndChild( decl );  
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_APPENDNODE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlElement* elem = new TiXmlElement( pstr->value() );
					file.LinkEndChild(elem);
					if (ex.hasParams(2))
					{
						BStruct* attr = static_cast<BStruct*>(ex.getParamImp( 1, BObjectImp::OTStruct ));
						if (attr)
						{
							for( BStruct::Contents::const_iterator citr = attr->contents().begin(), end = attr->contents().end(); citr != end; ++citr )
							{
								const string& name = (*citr).first;
								BObjectImp* ref = (*citr).second->impptr();
								if (ref->isa(BObjectImp::OTLong))
									elem->SetAttribute(name,static_cast<BLong*>(ref)->value());
								else if (ref->isa(BObjectImp::OTDouble))
									elem->SetDoubleAttribute(name,static_cast<Double*>(ref)->value());
								else
									elem->SetAttribute(name,ref->getStringRep());
							}
						}
					}
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_APPENDXMLCOMMENT:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlComment* comment = new TiXmlComment(pstr->value().c_str());
					file.LinkEndChild(comment);
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_REMOVENODE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlNode* child = file.FirstChild(pstr->value());
					if (child)
						return new BLong( file.RemoveChild(child) ? 1:0 );
					else
						return new BError( "Failed to find node" );
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_SAVEXML:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					const Package* outpkg;
					string path;
					if (!pkgdef_split( pstr->value(), ex.prog()->pkg, &outpkg, &path ))
						return new BError( "Error in filename descriptor" );

					if (path.find( ".." ) != string::npos)
						return new BError( "No parent path traversal please." );

					if (!HasWriteAccess( ex.prog()->pkg, outpkg, path ))
						return new BError( "Access denied" );

					string filepath;
					if (outpkg == NULL)
						filepath = path;
					else
						filepath = outpkg->dir() + path;

					return new BLong(file.SaveFile(filepath) ? 1:0);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_XMLTOSTRING:
			{
				string indent = "\t";
				if (ex.hasParams(1))
				{
					const String* pstr;
					if (ex.getStringParam( 0, pstr ))
						indent=pstr->value();
				}
				TiXmlPrinter printer;
				printer.SetIndent( indent.c_str() );

				file.Accept( &printer );
				return new String(printer.CStr());
			}

		default:
			return NULL;
	}
}

BObjectImp* BXMLfile::copy() const
{
	return new BXMLfile(_filename);
}

std::string BXMLfile::getStringRep() const
{
	if (file.Error())
	{
		OSTRINGSTREAM os;
		os << file.ErrorRow() << "," << file.ErrorCol() << ":" << file.ErrorDesc();
		return OSTRINGSTREAM_STR(os);
	}
	return _filename;
}

bool BXMLfile::isTrue() const
{
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
				return new BError( "Invalid parameter type" );
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
		case MTH_APPENDNODE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlElement* nodeelem = node->ToElement();
					TiXmlElement* elem = new TiXmlElement( pstr->value() );
					nodeelem->LinkEndChild(elem);
					if (ex.hasParams(2))
					{
						BStruct* attr = static_cast<BStruct*>(ex.getParamImp( 1, BObjectImp::OTStruct ));
						if (attr)
						{
							for( BStruct::Contents::const_iterator citr = attr->contents().begin(), end = attr->contents().end(); citr != end; ++citr )
							{
								const string& name = (*citr).first;
								BObjectImp* ref = (*citr).second->impptr();
								if (ref->isa(BObjectImp::OTLong))
									elem->SetAttribute(name,static_cast<BLong*>(ref)->value());
								else if (ref->isa(BObjectImp::OTDouble))
									elem->SetDoubleAttribute(name,static_cast<Double*>(ref)->value());
								else
									elem->SetAttribute(name,ref->getStringRep());
							}
						}
					}
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_APPENDXMLCOMMENT:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlComment* comment = new TiXmlComment(pstr->value().c_str());
					node->LinkEndChild(comment);
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_SETATTRIBUTE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				BStruct* attr = static_cast<BStruct*>(ex.getParamImp( 0, BObjectImp::OTStruct ));
				if (attr)
				{
					TiXmlElement* elem = node->ToElement();
					for( BStruct::Contents::const_iterator citr = attr->contents().begin(), end = attr->contents().end(); citr != end; ++citr )
					{
						const string& name = (*citr).first;
						BObjectImp* ref = (*citr).second->impptr();
						if (ref->isa(BObjectImp::OTLong))
							elem->SetAttribute(name,static_cast<BLong*>(ref)->value());
						else if (ref->isa(BObjectImp::OTDouble))
							elem->SetDoubleAttribute(name,static_cast<Double*>(ref)->value());
						else
							elem->SetAttribute(name,ref->getStringRep());
					}
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_REMOVEATTRIBUTE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlElement* elem = node->ToElement();
					elem->RemoveAttribute(pstr->value());
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_REMOVENODE:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlNode* child = node->FirstChild(pstr->value());
					if (child)
						return new BLong( node->RemoveChild(child) ? 1:0 );
					else
						return new BError( "Failed to find node" );
				}
				return new BError( "Invalid parameter type" );
			}
		case MTH_APPENDTEXT:
			{
				if (!ex.hasParams(1))
					return new BError( "Not enough parameters" );
				const String* pstr;
				if (ex.getStringParam( 0, pstr ))
				{
					TiXmlElement* elem = node->ToElement();
					elem->LinkEndChild( new TiXmlText( pstr->value() ) );
					return new BLong(1);
				}
				return new BError( "Invalid parameter type" );
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