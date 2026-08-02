/** @file
 *
 * @par History
 */


#ifndef XMLSCROBJ_H
#define XMLSCROBJ_H

#include "bscript/bobject.h"

#include <map>
#include <memory>
#include <string>
#include <tinyxml/tinyxml.h>

#include "bscript/bcontiter.h"
#include "clib/rawtypes.h"


namespace Pol::Bscript
{
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Core
{
// A xml tree and its owner: the document of a XMLFile or the detached node returned by clonenode.
// The script objects below only point into the tree, every one of them holds a reference so the
// tree stays alive as long as it is used, even if the XMLFile object itself is long gone.
// tinyxml deletes a node when it is removed from its parent, therefore a script object does not
// point at a node directly but at a handle, which is shared by all objects for the same node and
// is cleared when the node gets removed.
class XmlTree
{
public:
  using NodeRef = std::shared_ptr<TiXmlNode*>;

  explicit XmlTree( TiXmlNode* root ) : _root( root ), _handles() {}
  TiXmlNode* root() const { return _root.get(); }
  // the handle of a node of this tree, created on demand
  NodeRef handle( TiXmlNode* node );
  // removes a child from its parent, invalidating the handles of the deleted nodes
  bool removeChild( TiXmlNode* parent, TiXmlNode* child );

private:
  void invalidate( TiXmlNode* node );

  std::unique_ptr<TiXmlNode> _root;
  std::map<TiXmlNode*, std::weak_ptr<TiXmlNode*>> _handles;
};
using XmlTreeRef = std::shared_ptr<XmlTree>;
using XmlNodeRef = XmlTree::NodeRef;

class BXMLNodeIterator final : public Bscript::ContIterator
{
public:
  BXMLNodeIterator( XmlTreeRef tree, TiXmlDocument* file, Bscript::BObject* pIter );
  BXMLNodeIterator( XmlTreeRef tree, TiXmlNode* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  XmlTreeRef _tree;
  XmlNodeRef _node;
  TiXmlDocument* _file;
  bool _init;
  Bscript::BObjectRef m_IterVal;
  Bscript::BLong* m_pIterVal;
};

class BXMLAttributeIterator final : public Bscript::ContIterator
{
public:
  BXMLAttributeIterator( XmlTreeRef tree, TiXmlNode* node, Bscript::BObject* pIter );
  Bscript::BObject* step() override;

private:
  XmlTreeRef _tree;
  XmlNodeRef _node;
  std::string _attrib;  // name of the attribute of the last step, it is looked up again
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
    return new BXMLNodeIterator( _tree, _doc, pIterVal );
  }

private:
  XmlTreeRef _tree;
  TiXmlDocument* _doc;  // the root of _tree, valid as long as _tree is held
  std::string _filename;
};


class BXmlNode final : public Bscript::BObjectImp
{
public:
  // a node of a tree owned by someone else
  BXmlNode( XmlTreeRef tree, TiXmlNode* _node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ),
        _tree( tree ),
        _node( _tree->handle( _node ) )
  {
  }
  // the root of its own tree, eg the detached node returned by clonenode
  explicit BXmlNode( XmlTreeRef tree )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ),
        _tree( tree ),
        _node( _tree->handle( _tree->root() ) )
  {
  }
  ~BXmlNode() override = default;
  Bscript::BObjectImp* copy() const override { return new BXmlNode( _tree, _node ); }
  std::string getStringRep() const override;

  const char* typeOf() const override { return "XMLNode"; }
  u8 typeOfInt() const override { return OTXMLNode; }
  size_t sizeEstimate() const override { return sizeof( *this ) + sizeof( TiXmlNode ); }
  bool isTrue() const override { return getNode() != nullptr; }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLNodeIterator( _tree, getNode(), pIterVal );
  }

  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  // nullptr once the node was removed from its tree
  TiXmlNode* getNode() const { return *_node; }

private:
  BXmlNode( XmlTreeRef tree, XmlNodeRef node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLNode ), _tree( tree ), _node( node )
  {
  }

  XmlTreeRef _tree;
  XmlNodeRef _node;
};

class BXmlAttribute final : public Bscript::BObjectImp
{
public:
  BXmlAttribute( XmlTreeRef tree, TiXmlNode* _node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLAttributes ),
        _tree( tree ),
        _node( _tree->handle( _node ) )
  {
  }

  Bscript::BObjectImp* copy() const override { return new BXmlAttribute( _tree, _node ); }

  std::string getStringRep() const override { return "XMLAttributes"; }
  const char* typeOf() const override { return "XMLAttributes"; }
  u8 typeOfInt() const override { return OTXMLAttributes; }
  size_t sizeEstimate() const override { return sizeof( *this ); }
  bool isTrue() const override { return element() != nullptr; }
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override
  {
    return new BXMLAttributeIterator( _tree, *_node, pIterVal );
  }
  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;

private:
  BXmlAttribute( XmlTreeRef tree, XmlNodeRef node )
      : Bscript::BObjectImp( Bscript::BObjectImp::OTXMLAttributes ), _tree( tree ), _node( node )
  {
  }
  // nullptr once the element was removed from its tree
  TiXmlElement* element() const { return *_node ? ( *_node )->ToElement() : nullptr; }

  XmlTreeRef _tree;
  XmlNodeRef _node;
};
}  // namespace Pol::Core


#endif
