/** @file
 *
 * @par History
 */


#include "xmlfilescrobj.h"

#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/bstruct.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/stlutil.h"
#include "../plib/pkg.h"
#include "module/fileaccess.h"


namespace Pol::Core
{
using namespace Bscript;
BXMLfile::BXMLfile() : Bscript::BObjectImp( OTXMLFile ), file(), _filename( "" ) {}

BXMLfile::BXMLfile( std::string filename )
    : Bscript::BObjectImp( OTXMLFile ), file( filename.c_str() ), _filename( filename )
{
  if ( !file.LoadFile() )
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
      TiXmlDeclaration* decl =
          new TiXmlDeclaration( version->value(), encoding->value(), standalone->value() );
      if ( !file.NoChildren() )  // in case its not the first method used
      {
        if ( file.FirstChild()->Type() == TiXmlNode::TINYXML_DECLARATION )
          file.RemoveChild( file.FirstChild() );  // remove old declaration
        if ( !file.NoChildren() )
          file.InsertBeforeChild( file.FirstChild(), *decl );
        else
          file.LinkEndChild( decl );
      }
      else
        file.LinkEndChild( decl );
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
          for ( BStruct::Contents::const_iterator citr = attr->contents().begin(),
                                                  end = attr->contents().end();
                citr != end; ++citr )
          {
            const std::string& name = ( *citr ).first;
            Bscript::BObjectImp* ref = ( *citr ).second->impptr();
            if ( ref->isa( Bscript::BObjectImp::OTLong ) )
              elem->SetAttribute( name, static_cast<BLong*>( ref )->value() );
            else if ( ref->isa( Bscript::BObjectImp::OTDouble ) )
              elem->SetDoubleAttribute( name, static_cast<Double*>( ref )->value() );
            else
              elem->SetAttribute( name, ref->getStringRep() );
          }
        }
      }
      file.LinkEndChild( elem.release() );
      return new BXmlNode( file.LastChild() );
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
      file.LinkEndChild( comment );
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
      TiXmlNode* child = file.FirstChild( pstr->value() );
      if ( child )
        return new BLong( file.RemoveChild( child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    if ( imp->isa( Bscript::BObjectImp::OTLong ) )
    {
      const BLong* keyint = Clib::explicit_cast<BLong*, Bscript::BObjectImp*>( imp );
      if ( keyint->value() != 1 )
        return new BError( "Failed to find node" );
      return new BLong( file.RemoveChild( file.RootElement() ) ? 1 : 0 );
    }
    else if ( imp->isa( Bscript::BObjectImp::OTXMLNode ) )
    {
      const BXmlNode* pstr = Clib::explicit_cast<BXmlNode*, Bscript::BObjectImp*>( imp );
      TiXmlNode* node = file.ToElement();
      if ( node != pstr->getNode()->Parent() )
        return new BError( "Failed to find node" );
      return new BLong( file.RemoveChild( pstr->getNode() ) ? 1 : 0 );
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

      return new BLong( file.SaveFile( filepath ) ? 1 : 0 );
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

    file.Accept( &printer );
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
  if ( file.Error() )
  {
    OSTRINGSTREAM os;
    os << file.ErrorRow() << "," << file.ErrorCol() << ":" << file.ErrorDesc();
    return OSTRINGSTREAM_STR( os );
  }
  return _filename;
}

bool BXMLfile::isTrue() const
{
  return !file.Error();
}

BObjectRef BXMLfile::OperSubscript( const BObject& obj )
{
  if ( obj->isa( OTString ) )
  {
    const String* keystr = obj.impptr<String>();
    TiXmlNode* node = file.FirstChild( keystr->value() );
    if ( node )
      return BObjectRef( new BXmlNode( node ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  if ( obj->isa( OTLong ) )
  {
    BLong& keyint = (BLong&)obj.impref();
    TiXmlHandle handle( &file );
    TiXmlNode* node = handle.Child( keyint.value() - 1 )
                          .ToNode();  // keep escript 1based index and change it to 0based
    if ( node )
      return BObjectRef( new BXmlNode( node ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  else
  {
    return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
  }
}

BXmlNode::~BXmlNode()
{
  // a tiny hack: copy does copy the node just the pointer, only clonenode does, delete this cloned
  // node if its the last reference and tinyxml does not delete it via parent, clone flag will not
  // be transfered so only the first instance will be deleted
  if ( _cloned && node != nullptr && count() == 0 && node->Parent() == nullptr )
  {
    delete node;
  }
}

BObjectRef BXmlNode::get_member_id( const int id )  // id test
{
  switch ( id )
  {
  case MBR_ATTRIBUTES:
    if ( node->ToElement() )
      return BObjectRef( new BXmlAttribute( node ) );
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
          return new BXmlNode( child );
        return new BError( "Failed to find node" );
      }
      return new BError( "Invalid parameter type" );
    }
    else
    {
      TiXmlNode* child = node->FirstChild();
      if ( child )
        return new BXmlNode( child );
      return new BError( "Failed to find node" );
    }
    break;
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
          return new BXmlNode( sibling );
        return new BError( "Failed to find node" );
      }
    }
    else
    {
      TiXmlNode* sibling = node->NextSibling();
      if ( sibling )
        return new BXmlNode( sibling );
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
          for ( BStruct::Contents::const_iterator citr = attr->contents().begin(),
                                                  end = attr->contents().end();
                citr != end; ++citr )
          {
            const std::string& name = ( *citr ).first;
            Bscript::BObjectImp* ref = ( *citr ).second->impptr();
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
      nodeelem->LinkEndChild( elem.release() );
      return new BXmlNode( nodeelem->LastChild() );
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
      for ( BStruct::Contents::const_iterator citr = attr->contents().begin(),
                                              end = attr->contents().end();
            citr != end; ++citr )
      {
        const std::string& name = ( *citr ).first;
        Bscript::BObjectImp* ref = ( *citr ).second->impptr();
        if ( ref->isa( Bscript::BObjectImp::OTLong ) )
          elem->SetAttribute( name, static_cast<BLong*>( ref )->value() );
        else if ( ref->isa( Bscript::BObjectImp::OTDouble ) )
          elem->SetDoubleAttribute( name, static_cast<Double*>( ref )->value() );
        else
          elem->SetAttribute( name, ref->getStringRep() );
      }
      return new BXmlNode( elem );
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
      elem->RemoveAttribute( pstr->value() );
      return new BXmlNode( elem );
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
        return new BLong( node->RemoveChild( child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    if ( imp->isa( Bscript::BObjectImp::OTLong ) )
    {
      const BLong* keyint = Clib::explicit_cast<BLong*, Bscript::BObjectImp*>( imp );
      TiXmlHandle handle( node );
      TiXmlNode* child = handle.Child( keyint->value() - 1 )
                             .ToNode();  // keep escript 1based index and change it to 0based
      if ( child )
        return new BLong( node->RemoveChild( child ) ? 1 : 0 );
      return new BError( "Failed to find node" );
    }
    else if ( imp->isa( Bscript::BObjectImp::OTXMLNode ) )
    {
      const BXmlNode* pstr = Clib::explicit_cast<BXmlNode*, Bscript::BObjectImp*>( imp );
      if ( node->Parent() != pstr->getNode()->Parent() )
        return new BError( "Failed to find node" );
      return new BLong( node->RemoveChild( pstr->getNode() ) ? 1 : 0 );
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
      elem->LinkEndChild( new TiXmlText( pstr->value() ) );
      return new BXmlNode( elem->LastChild() );
    }
    break;
  }
  case MTH_CLONENODE:
  {
    return new BXmlNode( node->Clone(), true );
  }
  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}

BObjectRef BXmlNode::OperSubscript( const BObject& obj )
{
  if ( obj->isa( OTString ) )
  {
    const String* keystr = obj.impptr<String>();
    TiXmlNode* child = node->FirstChild( keystr->value() );
    if ( child )
      return BObjectRef( new BXmlNode( child ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  if ( obj->isa( OTLong ) )
  {
    BLong& keyint = (BLong&)obj.impref();
    TiXmlHandle handle( node );
    TiXmlNode* child = handle.Child( keyint.value() - 1 )
                           .ToNode();  // keep escript 1based index and change it to 0based
    if ( child )
      return BObjectRef( new BXmlNode( child ) );
    return BObjectRef( new BError( "Failed to find node" ) );
  }
  else
  {
    return BObjectRef( new BError( "xml members can only be accessed by name or index" ) );
  }
}

std::string BXmlNode::getStringRep() const
{
  if ( node->Type() == TiXmlNode::TINYXML_TEXT )
  {
    std::string text = node->ToText()->Value();
    Clib::sanitizeUnicodeWithIso( &text );
    return text;
  }
  if ( node->Type() == TiXmlNode::TINYXML_DECLARATION )
  {
    TiXmlDeclaration* dec = node->ToDeclaration();
    OSTRINGSTREAM os;
    os << "v:" << dec->Version() << " e:" << dec->Encoding() << " s:" << dec->Standalone();
    return OSTRINGSTREAM_STR( os );
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

BXMLNodeIterator::BXMLNodeIterator( TiXmlDocument* file, BObject* pIter )
    :  // root elements
      node( nullptr ),
      _file( file ),
      _init( false ),
      m_IterVal( pIter ),
      m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}
BXMLNodeIterator::BXMLNodeIterator( TiXmlNode* _node, BObject* pIter )
    :  // child elements
      node( _node ),
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
  if ( node == nullptr )  // root elements (iter over BXmlFile)
    node = _file->FirstChild();
  else if ( _file == nullptr )  // child elements (iter over BXmlNode)
  {
    if ( !_init )
    {
      _init = true;
      node = node->FirstChild();
    }
    else
      node = node->NextSiblingElement();
  }
  else  // root elements (iter over BXmlFile)
    node = node->NextSibling();
  if ( !node )
    return nullptr;

  return new BObject( new BXmlNode( node ) );
}

BXMLAttributeIterator::BXMLAttributeIterator( TiXmlElement* _node, BObject* pIter )
    : node( _node ), nodeAttrib( nullptr ), m_IterVal( pIter ), m_pIterVal( new BLong( 0 ) )
{
  m_IterVal.get()->setimp( m_pIterVal );
}

BObject* BXMLAttributeIterator::step()
{
  m_pIterVal->increment();
  if ( !node )
    return nullptr;
  if ( nodeAttrib == nullptr )
    nodeAttrib = node->FirstAttribute();
  else
    nodeAttrib = nodeAttrib->Next();
  if ( nodeAttrib )
  {
    std::unique_ptr<BStruct> details( new BStruct() );
    std::string name = nodeAttrib->Name();
    Clib::sanitizeUnicodeWithIso( &name );
    details->addMember( name.c_str(), new String( nodeAttrib->Value(), String::Tainted::YES ) );
    return new BObject( details.release() );
  }
  return nullptr;
}
}  // namespace Pol::Core
