/** @file
 *
 * @par History
 */


#include "pol/xmlfilescrobj.h"

#include <stddef.h>

#include "bscript/barray.h"
#include "bscript/bdouble.h"
#include "bscript/berror.h"
#include "bscript/blong.h"
#include "bscript/bstring.h"
#include "bscript/bstruct.h"
#include "bscript/buninit.h"
#include "bscript/executor.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"
#include "plib/pkg.h"

#include "pol/module/fileaccess.h"


namespace Pol::Core
{
using namespace Bscript;

XmlTree::NodeRef XmlTree::handle( TiXmlNode* node )
{
  if ( node == nullptr )  // an already removed node, its handle is not shared
    return std::make_shared<TiXmlNode*>( nullptr );
  auto itr = _handles.find( node );
  if ( itr != _handles.end() )
  {
    if ( auto existing = itr->second.lock() )
      return existing;
    _handles.erase( itr );
  }
  auto created = std::make_shared<TiXmlNode*>( node );
  _handles[node] = created;
  return created;
}

void XmlTree::invalidate( TiXmlNode* node )
{
  for ( TiXmlNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling() )
    invalidate( child );
  auto itr = _handles.find( node );
  if ( itr == _handles.end() )
    return;
  if ( auto existing = itr->second.lock() )
    *existing = nullptr;
  _handles.erase( itr );
}

bool XmlTree::removeChild( TiXmlNode* parent, TiXmlNode* child )
{
  if ( child == nullptr )
    return false;
  invalidate( child );  // tinyxml deletes the node and its children
  return parent->RemoveChild( child );
}

BXMLfile::BXMLfile()
    : Bscript::BObjectImp( OTXMLFile ),
      _tree( std::make_shared<XmlTree>( new TiXmlDocument() ) ),
      _doc( static_cast<TiXmlDocument*>( _tree->root() ) ),
      _filename( "" )
{
}

BXMLfile::BXMLfile( std::string filename )
    : Bscript::BObjectImp( OTXMLFile ),
      _tree( std::make_shared<XmlTree>( new TiXmlDocument( filename.c_str() ) ) ),
      _doc( static_cast<TiXmlDocument*>( _tree->root() ) ),
      _filename( filename )
{
  if ( !_doc->LoadFile() )
    return;
}

BObjectRef BXMLfile::get_member_id( const int /*id*/ )  // id test
{
  return BObjectRef( UninitObject::create() );
  // switch(id)
  //{

  //  default: return BObjectRef(UninitObject::create());
  //}
}
BObjectRef BXMLfile::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

Bscript::BObjectImp* BXMLfile::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  return nullptr;
}

Bscript::BObjectImp* BXMLfile::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_SETDECLARATION:
  {
    if ( !ex.hasParams( 3 ) )
      return new BError( "Not enough parameters" );
    const String* version;
    const String* encoding;
    const String* standalone;
    if ( ex.getStringParam( 0, version ) && ex.getStringParam( 1, encoding ) &&
         ex.getStringParam( 2, standalone ) )
    {
      std::unique_ptr<TiXmlDeclaration> decl(
          new TiXmlDeclaration( version->value(), encoding->value(), standalone->value() ) );
      if ( !_doc->NoChildren() )  // in case its not the first method used
      {
        if ( _doc->FirstChild()->Type() == TiXmlNode::TINYXML_DECLARATION )
          _tree->removeChild( _doc, _doc->FirstChild() );  // remove old declaration
        if ( !_doc->NoChildren() )
          _doc->InsertBeforeChild( _doc->FirstChild(), *decl );  // inserts a copy
        else
          _doc->LinkEndChild( decl.release() );
      }
      else
        _doc->LinkEndChild( decl.release() );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_APPENDNODE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      std::unique_ptr<TiXmlElement> elem( new TiXmlElement( pstr->value() ) );

      if ( ex.hasParams( 2 ) )
      {
        BStruct* attr = static_cast<BStruct*>( ex.getParamImp( 1, Bscript::BObjectImp::OTStruct ) );
        if ( attr )
        {
          for ( const auto& citr : attr->contents() )
          {
            const std::string& name = citr.first;
            Bscript::BObjectImp* ref = citr.second->impptr();
            if ( ref->isa( Bscript::BObjectImp::OTLong ) )
              elem->SetAttribute( name, static_cast<BLong*>( ref )->value() );
            else if ( ref->isa( Bscript::BObjectImp::OTDouble ) )
              elem->SetDoubleAttribute( name, static_cast<Double*>( ref )->value() );
            else
              elem->SetAttribute( name, ref->getStringRep() );
          }
        }
      }
      _doc->LinkEndChild( elem.release() );
      return new BXmlNode( _tree, _doc->LastChild() );
    }
    break;
  }
  case MTH_APPENDXMLCOMMENT:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      TiXmlComment* comment = new TiXmlComment( pstr->value().c_str() );
      _doc->LinkEndChild( comment );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_REMOVENODE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    Bscript::BObjectImp* imp = ex.getParamImp( 0 );
    if ( imp->isa( Bscript::BObjectImp::OTString ) )
    {
      const String* pstr = Clib::explicit_cast<String*, Bscript::BObjectImp*>( imp );
      TiXmlNode* child = _doc->FirstChild( pstr->value() );
      if ( child )
        return new BLong( _tree->removeChild( _doc, child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    if ( imp->isa( Bscript::BObjectImp::OTLong ) )
    {
      const BLong* keyint = Clib::explicit_cast<BLong*, Bscript::BObjectImp*>( imp );
      if ( keyint->value() != 1 )
        return new BError( "Failed to find node" );
      return new BLong( _tree->removeChild( _doc, _doc->RootElement() ) ? 1 : 0 );
    }
    if ( imp->isa( Bscript::BObjectImp::OTXMLNode ) )
    {
      const BXmlNode* pstr = Clib::explicit_cast<BXmlNode*, Bscript::BObjectImp*>( imp );
      if ( pstr->getNode() == nullptr || pstr->getNode()->Parent() != _doc )
        return new BError( "Failed to find node" );
      return new BLong( _tree->removeChild( _doc, pstr->getNode() ) ? 1 : 0 );
    }
    break;
  }
  case MTH_SAVEXML:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      const Plib::Package* outpkg;
      std::string path;
      if ( !pkgdef_split( pstr->value(), ex.prog()->pkg, &outpkg, &path ) )
        return new BError( "Error in filename descriptor" );

      if ( path.find( ".." ) != std::string::npos )
        return new BError( "No parent path traversal please." );

      if ( !Module::HasWriteAccess( ex.prog()->pkg, outpkg, path ) )
        return new BError( "Access denied" );

      std::string filepath;
      if ( outpkg == nullptr )
        filepath = path;
      else
        filepath = outpkg->dir() + path;

      return new BLong( _doc->SaveFile( filepath ) ? 1 : 0 );
    }
    break;
  }
  case MTH_XMLTOSTRING:
  {
    std::string indent = "\t";
    if ( ex.hasParams( 1 ) )
    {
      const String* pstr;
      if ( ex.getStringParam( 0, pstr ) )
        indent = pstr->value();
    }
    TiXmlPrinter printer;
    printer.SetIndent( indent.c_str() );

    _doc->Accept( &printer );
    return new String( printer.CStr(), String::Tainted::YES );
  }

  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* BXMLfile::copy() const
{
  return new BXMLfile( _filename );
}

std::string BXMLfile::getStringRep() const
{
  if ( _doc->Error() )
  {
    return fmt::format( "{},{}:{}", _doc->ErrorRow(), _doc->ErrorCol(), _doc->ErrorDesc() );
  }
  return _filename;
}

bool BXMLfile::isTrue() const
{
  return !_doc->Error();
}

BObjectRef BXMLfile::OperSubscript( const BObject& obj )
{
  if ( obj->isa( OTString ) )
  {
    const String* keystr = obj.impptr<String>();
    TiXmlNode* node = _doc->FirstChild( keystr->value() );
    if ( node )
      return BObjectRef( new BXmlNode( _tree, node ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  if ( obj->isa( OTLong ) )
  {
    BLong& keyint = (BLong&)obj.impref();
    TiXmlHandle handle( _doc );
    TiXmlNode* node = handle.Child( keyint.value() - 1 )
                          .ToNode();  // keep escript 1based index and change it to 0based
    if ( node )
      return BObjectRef( new BXmlNode( _tree, node ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }

  return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
}

BObjectRef BXmlNode::get_member_id( const int id )  // id test
{
  TiXmlNode* node = getNode();
  if ( node == nullptr )
    return BObjectRef( new BError( "Node was removed" ) );
  switch ( id )
  {
  case MBR_ATTRIBUTES:
    if ( node->ToElement() )
      return BObjectRef( new BXmlAttribute( _tree, node ) );
    else
      return BObjectRef( new BError( "No attributes available." ) );

  case MBR_TYPE:
    switch ( node->Type() )
    {
    case ( TiXmlNode::TINYXML_COMMENT ):
      return BObjectRef( new String( "XMLComment" ) );
    case ( TiXmlNode::TINYXML_DECLARATION ):
      return BObjectRef( new String( "XMLDeclaration" ) );
    case ( TiXmlNode::TINYXML_DOCUMENT ):
      return BObjectRef( new String( "XMLDocument" ) );
    case ( TiXmlNode::TINYXML_ELEMENT ):
      return BObjectRef( new String( "XMLElement" ) );
    case ( TiXmlNode::TINYXML_TEXT ):
      return BObjectRef( new String( "XMLText" ) );
    case ( TiXmlNode::TINYXML_TYPECOUNT ):
      return BObjectRef( new String( "XMLTypeCount" ) );
    case ( TiXmlNode::TINYXML_UNKNOWN ):
      return BObjectRef( new String( "XMLUnknown" ) );
    default:
      return BObjectRef( new String( "XMLUnknown" ) );
    }

  default:
    return BObjectRef( UninitObject::create() );
  }
}
BObjectRef BXmlNode::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

Bscript::BObjectImp* BXmlNode::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  return nullptr;
}

Bscript::BObjectImp* BXmlNode::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  TiXmlNode* node = getNode();
  if ( node == nullptr )
    return new BError( "Node was removed" );
  switch ( id )
  {
  case MTH_FIRSTCHILD:
  {
    if ( ex.hasParams( 1 ) )
    {
      const String* pstr;
      if ( ex.getStringParam( 0, pstr ) )
      {
        TiXmlNode* child = node->FirstChild( pstr->value() );
        if ( child )
          return new BXmlNode( _tree, child );
        return new BError( "Failed to find node" );
      }
      return new BError( "Invalid parameter type" );
    }
    TiXmlNode* child = node->FirstChild();
    if ( child )
      return new BXmlNode( _tree, child );
    return new BError( "Failed to find node" );
  }
  case MTH_NEXTSIBLING:
  {
    if ( ex.hasParams( 1 ) )
    {
      const String* pstr;
      if ( ex.getStringParam( 0, pstr ) )
      {
        TiXmlNode* sibling = node->NextSibling( pstr->value() );
        if ( sibling )
          return new BXmlNode( _tree, sibling );
        return new BError( "Failed to find node" );
      }
    }
    else
    {
      TiXmlNode* sibling = node->NextSibling();
      if ( sibling )
        return new BXmlNode( _tree, sibling );
      return new BError( "Failed to find node" );
    }
    break;
  }
  case MTH_APPENDNODE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      std::unique_ptr<TiXmlElement> elem( new TiXmlElement( pstr->value() ) );

      if ( ex.hasParams( 2 ) )
      {
        BStruct* attr = static_cast<BStruct*>( ex.getParamImp( 1, Bscript::BObjectImp::OTStruct ) );
        if ( attr )
        {
          for ( const auto& citr : attr->contents() )
          {
            const std::string& name = citr.first;
            Bscript::BObjectImp* ref = citr.second->impptr();
            if ( ref->isa( Bscript::BObjectImp::OTLong ) )
              elem->SetAttribute( name, static_cast<BLong*>( ref )->value() );
            else if ( ref->isa( Bscript::BObjectImp::OTDouble ) )
              elem->SetDoubleAttribute( name, static_cast<Double*>( ref )->value() );
            else
              elem->SetAttribute( name, ref->getStringRep() );
          }
        }
      }

      TiXmlElement* nodeelem = node->ToElement();
      if ( !nodeelem )
        return new BError( "Node is not an element" );
      nodeelem->LinkEndChild( elem.release() );
      return new BXmlNode( _tree, nodeelem->LastChild() );
    }
    break;
  }
  case MTH_APPENDXMLCOMMENT:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      TiXmlComment* comment = new TiXmlComment( pstr->value().c_str() );
      node->LinkEndChild( comment );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_SETATTRIBUTE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    BStruct* attr = static_cast<BStruct*>( ex.getParamImp( 0, Bscript::BObjectImp::OTStruct ) );
    if ( attr )
    {
      TiXmlElement* elem = node->ToElement();
      if ( !elem )
        return new BError( "Node is not an element" );
      for ( const auto& citr : attr->contents() )
      {
        const std::string& name = citr.first;
        Bscript::BObjectImp* ref = citr.second->impptr();
        if ( ref->isa( Bscript::BObjectImp::OTLong ) )
          elem->SetAttribute( name, static_cast<BLong*>( ref )->value() );
        else if ( ref->isa( Bscript::BObjectImp::OTDouble ) )
          elem->SetDoubleAttribute( name, static_cast<Double*>( ref )->value() );
        else
          elem->SetAttribute( name, ref->getStringRep() );
      }
      return new BXmlNode( _tree, elem );
    }
    break;
  }
  case MTH_REMOVEATTRIBUTE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      TiXmlElement* elem = node->ToElement();
      if ( !elem )
        return new BError( "Node is not an element" );
      elem->RemoveAttribute( pstr->value() );
      return new BXmlNode( _tree, elem );
    }
    break;
  }
  case MTH_REMOVENODE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    Bscript::BObjectImp* imp = ex.getParamImp( 0 );
    if ( imp->isa( Bscript::BObjectImp::OTString ) )
    {
      const String* pstr = Clib::explicit_cast<String*, Bscript::BObjectImp*>( imp );
      TiXmlNode* child = node->FirstChild( pstr->value() );
      if ( child )
        return new BLong( _tree->removeChild( node, child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    if ( imp->isa( Bscript::BObjectImp::OTLong ) )
    {
      const BLong* keyint = Clib::explicit_cast<BLong*, Bscript::BObjectImp*>( imp );
      TiXmlHandle handle( node );
      TiXmlNode* child = handle.Child( keyint->value() - 1 )
                             .ToNode();  // keep escript 1based index and change it to 0based
      if ( child )
        return new BLong( _tree->removeChild( node, child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    if ( imp->isa( Bscript::BObjectImp::OTXMLNode ) )
    {
      const BXmlNode* pstr = Clib::explicit_cast<BXmlNode*, Bscript::BObjectImp*>( imp );
      // only a direct child of this node can be removed
      if ( pstr->getNode() == nullptr || pstr->getNode()->Parent() != node )
        return new BError( "Failed to find node" );
      return new BLong( _tree->removeChild( node, pstr->getNode() ) ? 1 : 0 );
    }
    break;
  }
  case MTH_APPENDTEXT:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      TiXmlElement* elem = node->ToElement();
      if ( !elem )
        return new BError( "Node is not an element" );
      elem->LinkEndChild( new TiXmlText( pstr->value() ) );
      return new BXmlNode( _tree, elem->LastChild() );
    }
    break;
  }
  case MTH_CLONENODE:
  {
    return new BXmlNode( std::make_shared<XmlTree>( node->Clone() ) );
  }
  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}

BObjectRef BXmlNode::OperSubscript( const BObject& obj )
{
  TiXmlNode* node = getNode();
  if ( node == nullptr )
    return BObjectRef( new BError( "Node was removed" ) );
  if ( obj->isa( OTString ) )
  {
    const String* keystr = obj.impptr<String>();
    TiXmlNode* child = node->FirstChild( keystr->value() );
    if ( child )
      return BObjectRef( new BXmlNode( _tree, child ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  if ( obj->isa( OTLong ) )
  {
    BLong& keyint = (BLong&)obj.impref();
    TiXmlHandle handle( node );
    TiXmlNode* child = handle.Child( keyint.value() - 1 )
                           .ToNode();  // keep escript 1based index and change it to 0based
    if ( child )
      return BObjectRef( new BXmlNode( _tree, child ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }

  return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
}

std::string BXmlNode::getStringRep() const
{
  TiXmlNode* node = getNode();
  if ( node == nullptr )
    return "removed XMLNode";
  if ( node->Type() == TiXmlNode::TINYXML_TEXT )
  {
    std::string text = node->ToText()->Value();
    Clib::sanitizeUnicodeWithIso( &text );
    return text;
  }
  if ( node->Type() == TiXmlNode::TINYXML_DECLARATION )
  {
    TiXmlDeclaration* dec = node->ToDeclaration();
    return fmt::format( "v:{} e:{} s:{}", dec->Version(), dec->Encoding(), dec->Standalone() );
  }
  std::string text = node->Value();
  Clib::sanitizeUnicodeWithIso( &text );
  return text;
}

Bscript::BObjectImp* BXmlAttribute::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  return nullptr;
}

Bscript::BObjectImp* BXmlAttribute::call_method_id( const int id, Executor& /*ex*/,
                                                    bool /*forcebuiltin*/ )
{
  TiXmlElement* node = element();
  if ( !node )
    return nullptr;
  switch ( id )
  {
  case MTH_PROPNAMES:
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    TiXmlAttribute* attrib = node->FirstAttribute();
    while ( attrib )
    {
      arr->addElement( new String( attrib->Name(), String::Tainted::YES ) );
      attrib = attrib->Next();
    }
    return arr.release();
  }
  default:
    return nullptr;
  }
}

BObjectRef BXmlAttribute::OperSubscript( const BObject& obj )
{
  TiXmlElement* node = element();
  if ( node == nullptr )
    return BObjectRef( new BError( "Node was removed" ) );
  if ( obj->isa( OTString ) )
  {
    const String* keystr = obj.impptr<String>();
    const std::string* attrib = node->Attribute( keystr->value() );
    if ( attrib )
      return BObjectRef( new String( attrib->c_str(), String::Tainted::YES ) );
    return BObjectRef( new BError( "Failed to find attribute" ) );
  }

  return BObjectRef( new BError( "xml attribute can only be accessed by name" ) );
}

BXMLNodeIterator::BXMLNodeIterator( XmlTreeRef tree, TiXmlDocument* file, BObject* pIter )
    :  // root elements
      _tree( tree ),
      _node(),
      _file( file ),
      _init( false ),
      m_IterVal( pIter ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}
BXMLNodeIterator::BXMLNodeIterator( XmlTreeRef tree, TiXmlNode* node, BObject* pIter )
    :  // child elements
      _tree( tree ),
      _node( _tree->handle( node ) ),
      _file( nullptr ),
      _init( false ),
      m_IterVal( pIter ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}

BObject* BXMLNodeIterator::step()
{
  m_pIterVal->increment();
  TiXmlNode* node;
  if ( !_node )  // first step of the iteration over a BXmlFile
    node = _file->FirstChild();
  else if ( *_node == nullptr )  // the node the iteration stands on was removed
    return nullptr;
  else if ( _file == nullptr )  // child elements (iter over BXmlNode)
  {
    if ( !_init )
    {
      _init = true;
      node = ( *_node )->FirstChild();
    }
    else
      node = ( *_node )->NextSiblingElement();
  }
  else  // root elements (iter over BXmlFile)
    node = ( *_node )->NextSibling();
  if ( !node )
    return nullptr;

  _node = _tree->handle( node );
  return new BObject( new BXmlNode( _tree, node ) );
}

BXMLAttributeIterator::BXMLAttributeIterator( XmlTreeRef tree, TiXmlNode* node, BObject* pIter )
    : _tree( tree ),
      _node( _tree->handle( node ) ),
      _attrib(),
      m_IterVal( pIter ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}

BObject* BXMLAttributeIterator::step()
{
  m_pIterVal->increment();
  TiXmlElement* node = *_node != nullptr ? ( *_node )->ToElement() : nullptr;
  if ( !node )  // no element or it was removed
    return nullptr;
  TiXmlAttribute* attrib = node->FirstAttribute();
  if ( !_attrib.empty() )
  {
    // the attribute of the last step is looked up again, it could have been removed meanwhile
    while ( attrib != nullptr && _attrib != attrib->Name() )
      attrib = attrib->Next();
    if ( attrib == nullptr )
      return nullptr;
    attrib = attrib->Next();
  }
  if ( attrib == nullptr )
    return nullptr;

  _attrib = attrib->Name();
  std::unique_ptr<BStruct> details( new BStruct() );
  std::string name = _attrib;
  Clib::sanitizeUnicodeWithIso( &name );
  details->addMember( name.c_str(), new String( attrib->Value(), String::Tainted::YES ) );
  return new BObject( details.release() );
}
}  // namespace Pol::Core
