
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_tree_DefaultMutableTreeNode__
#define __javax_swing_tree_DefaultMutableTreeNode__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace tree
      {
          class DefaultMutableTreeNode;
          class MutableTreeNode;
          class TreeNode;
      }
    }
  }
}

class javax::swing::tree::DefaultMutableTreeNode : public ::java::lang::Object
{

public:
  DefaultMutableTreeNode();
  DefaultMutableTreeNode(::java::lang::Object *);
  DefaultMutableTreeNode(::java::lang::Object *, jboolean);
  virtual ::java::lang::Object * clone();
  virtual ::java::lang::String * toString();
  virtual void add(::javax::swing::tree::MutableTreeNode *);
  virtual ::javax::swing::tree::TreeNode * getParent();
  virtual void remove(jint);
  virtual void remove(::javax::swing::tree::MutableTreeNode *);
private:
  void writeObject(::java::io::ObjectOutputStream *);
  void readObject(::java::io::ObjectInputStream *);
public:
  virtual void insert(::javax::swing::tree::MutableTreeNode *, jint);
  virtual JArray< ::javax::swing::tree::TreeNode * > * getPath();
  virtual ::java::util::Enumeration * children();
  virtual void setParent(::javax::swing::tree::MutableTreeNode *);
  virtual ::javax::swing::tree::TreeNode * getChildAt(jint);
  virtual jint getChildCount();
  virtual jint getIndex(::javax::swing::tree::TreeNode *);
  virtual void setAllowsChildren(jboolean);
  virtual jboolean getAllowsChildren();
  virtual void setUserObject(::java::lang::Object *);
  virtual ::java::lang::Object * getUserObject();
  virtual void removeFromParent();
  virtual void removeAllChildren();
  virtual jboolean isNodeAncestor(::javax::swing::tree::TreeNode *);
  virtual jboolean isNodeDescendant(::javax::swing::tree::DefaultMutableTreeNode *);
  virtual ::javax::swing::tree::TreeNode * getSharedAncestor(::javax::swing::tree::DefaultMutableTreeNode *);
  virtual jboolean isNodeRelated(::javax::swing::tree::DefaultMutableTreeNode *);
  virtual jint getDepth();
  virtual jint getLevel();
public: // actually protected
  virtual JArray< ::javax::swing::tree::TreeNode * > * getPathToRoot(::javax::swing::tree::TreeNode *, jint);
public:
  virtual JArray< ::java::lang::Object * > * getUserObjectPath();
  virtual ::javax::swing::tree::TreeNode * getRoot();
  virtual jboolean isRoot();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getNextNode();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getPreviousNode();
  virtual ::java::util::Enumeration * preorderEnumeration();
  virtual ::java::util::Enumeration * postorderEnumeration();
  virtual ::java::util::Enumeration * breadthFirstEnumeration();
  virtual ::java::util::Enumeration * depthFirstEnumeration();
  virtual ::java::util::Enumeration * pathFromAncestorEnumeration(::javax::swing::tree::TreeNode *);
  virtual jboolean isNodeChild(::javax::swing::tree::TreeNode *);
  virtual ::javax::swing::tree::TreeNode * getFirstChild();
  virtual ::javax::swing::tree::TreeNode * getLastChild();
  virtual ::javax::swing::tree::TreeNode * getChildAfter(::javax::swing::tree::TreeNode *);
  virtual ::javax::swing::tree::TreeNode * getChildBefore(::javax::swing::tree::TreeNode *);
  virtual jboolean isNodeSibling(::javax::swing::tree::TreeNode *);
  virtual jint getSiblingCount();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getNextSibling();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getPreviousSibling();
  virtual jboolean isLeaf();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getFirstLeaf();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getLastLeaf();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getNextLeaf();
  virtual ::javax::swing::tree::DefaultMutableTreeNode * getPreviousLeaf();
  virtual jint getLeafCount();
private:
  static const jlong serialVersionUID = -4298474751201349152LL;
public:
  static ::java::util::Enumeration * EMPTY_ENUMERATION;
public: // actually protected
  ::javax::swing::tree::MutableTreeNode * __attribute__((aligned(__alignof__( ::java::lang::Object)))) parent;
  ::java::util::Vector * children__;
  ::java::lang::Object * userObject;
  jboolean allowsChildren;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_tree_DefaultMutableTreeNode__
