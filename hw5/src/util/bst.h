/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
  // TODO: design your own class!!
  friend class BSTree<T>;
  friend class BSTree<T>::iterator;

  BSTreeNode(const T& b, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0, BSTreeNode<T>* p = 0):
  _data(b), _lnode(l), _rnode(r), _parent(p) {}

  //data member
  T               _data;
  BSTreeNode<T>*  _lnode;
  BSTreeNode<T>*  _rnode;
  BSTreeNode<T>*  _parent;
};


template <class T>
class BSTree
{
public:
  BSTree() {
    _tail = new BSTreeNode<T>(T());
    _tail->_parent = _tail->_rnode = _tail->_lnode = _tail;
    _root = _tail;
  }
  ~BSTree() {clear(); delete _root;}
  // TODO: design your own class!!
  class iterator
  {
    friend class BSTree;

  public:
    iterator(BSTreeNode<T>* n= 0): _node(n) {}
    iterator(const iterator& i) : _node(i._node) {}
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators
    const T& operator * () const { return *(this); }
    T& operator * () { return _node->_data; }
    iterator& operator ++ () //++b, find successor
    {
      if (_node->_lnode == _node)
      {
        _node = _node->_rnode;
        return (*this);
      }
      else if (_node->_rnode)
      {
        _node = _node->_rnode;
        if (_node->_lnode != _node)
        {
          while (_node->_lnode) _node = _node -> _lnode;
        }
        return (*this);
      }
      else
      {
        while (_node->_parent && _node == (_node->_parent)->_rnode) _node = _node->_parent;
        _node = _node->_parent;
        return (*this);
      }
    }
    iterator operator ++ (int) //b++
    {
      iterator temp = (*this);
      ++(*this);
      return temp;
    }
    iterator& operator -- ()
    {
      if (_node->_lnode == _node)
      {
        _node = _node->_parent;
        return (*this);
      }
      else if (_node->_lnode)
      {
        _node = _node->_lnode;
        if (_node->_lnode != _node)
        {
            while (_node->_rnode) _node = _node->_rnode;            
        }
        return (*this);
      }
      else
      {
        while (_node->_parent && _node == (_node->_parent)->_lnode) _node = _node->_parent;
        _node = _node->_parent;
        return (*this);
      }
    }
    iterator operator -- (int)
    {
      iterator temp = (*this);
      --(*this);
      return temp;
    }

    iterator& operator = (const iterator& i)
    {
      _node = i._node;
      return *(this);
    }

    bool operator != (const iterator& i) const { return (_node != i._node); }
    bool operator == (const iterator& i) const { return (_node == i._node); }
  private:
    BSTreeNode<T>* _node;

  };
  iterator begin() const //leftmost element, end() if empty()
  {
    return iterator(_tail->_rnode);
  }
  iterator end() const //handle it youself(but '--' bring back last element)
  {
    return iterator(_tail);
    /*
    if (empty()) return iterator(_root);
    else
    {
      iterator cur = iterator(_root);
      while ((cur._node)->_rnode) cur._node = (cur._node)->_rnode;
      return cur;
    }
    */
  }
  bool empty() const { return (_root == _tail); }
  size_t size() const
  {
    size_t n = 0;
    for (iterator it = begin(); it != end(); ++it) ++n;
    return n;
  }

  void insert(const T& x)
  {
    if (empty())
    {
      _root = new BSTreeNode<T>(x, _tail, _tail, 0);
      _tail->_rnode = _tail->_parent = _root;
    }
    else if (_root->_lnode == _root->_rnode)
    {
      if (x >= _root->_data)
      {
        BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, _tail, _root);
        _root->_rnode = b;
        _tail->_parent = b;
      }
      else
      {
        BSTreeNode<T>* b = new BSTreeNode<T>(x, _tail, 0, _root);
        _root->_lnode = b;
        _tail->_rnode = b;
      }
    }
    else
    {
      BSTreeNode<T>* cur = _root, *prev = _root;
      while (cur && cur->_lnode != cur)
      {
        prev = cur;
        if (x >= cur->_data) cur = cur->_rnode;
        else cur = cur->_lnode;
      }
      if (!cur)
      {
        BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, 0, prev);
        if (x >= prev->_data) prev->_rnode = b;
        else prev->_lnode = b;
      }
      else
      {
        if (prev == cur->_rnode)
        {
          BSTreeNode<T>* b = new BSTreeNode<T>(x, _tail, 0, prev);
          prev->_lnode = b;
          _tail->_rnode = b;
        }
        else
        {
          BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, _tail, prev);
          prev->_rnode = b;
          _tail->_parent = b;
        }

      }
    }
  }
  void pop_front() //remove left most node
  {
    if (empty()) return;
    erase(begin());
  }
  void pop_back() //remove right most node
  {
    if (empty()) return;
    erase(--end());
  }

  // return false if nothing to erase
  bool erase(iterator pos) //erase node at the pos
  {
    if (empty()) return false;
    else
    {
      if (pos == begin() && pos == (--end())) //only one element
      {
        delete pos._node;
        _tail->_parent = _tail->_rnode = _tail->_lnode = _tail;
        _root = _tail;
      }
      else if (pos == begin()) //leftmost
      {
        if (pos._node == _root)
        {
          _tail->_rnode = pos._node->_rnode;
          (pos._node->_rnode)->_parent = 0;
          _root = pos._node->_rnode;
          delete pos._node;
        }
        else if (pos._node->_rnode) //exist rightnode
        {
          BSTreeNode<T>* leftmost = pos._node;
          ++pos;
          (leftmost->_parent)->_lnode = leftmost->_rnode;
          (leftmost->_rnode)->_parent = leftmost->_parent;
          _tail->_rnode = pos._node;
          pos._node->_lnode = _tail;
          delete leftmost;
        }
        else //no rightnode
        {
          _tail->_rnode = pos._node->_parent;
          (pos._node->_parent)->_lnode = _tail;
          delete pos._node;
        }
      }
      else if (pos == (--end())) //rightmost
      {
        if (pos._node == _root)
        {
          _tail->_parent = pos._node->_lnode;
          (pos._node->_lnode)->_parent = 0;
          _root = pos._node->_lnode;
          delete pos._node;
        }
        else if (pos._node->_lnode) //exist leftnode
        {
          BSTreeNode<T>* rightmost = pos._node;
          --pos;
          (rightmost->_parent)->_rnode = rightmost->_lnode;
          (rightmost->_lnode)->_parent = rightmost->_parent;
          _tail->_parent = pos._node;
          pos._node->_rnode = _tail;
          delete rightmost;
        }
        else //no leftnode
        {
          _tail->_parent = pos._node->_parent;
          (pos._node->_parent)->_rnode = _tail;
          delete pos._node;
        }
      }
      else if (pos._node->_lnode && pos._node->_rnode) //two child
      {
        BSTreeNode<T>* cur = pos._node;
        ++pos;
        swap(cur, pos._node); //????
        erase(pos);
      } 
      else if (pos._node->_lnode || pos._node->_rnode) //one child
      {
        if ((pos._node->_parent)->_lnode == pos._node)
        {
          if (pos._node->_lnode)
          {
            (pos._node->_parent)->_lnode = pos._node->_lnode;
            (pos._node->_lnode)->_parent = pos._node->_parent;
          }
          else
          {
            (pos._node->_parent)->_lnode = pos._node->_rnode;
            (pos._node->_rnode)->_parent = pos._node->_parent;
          }
        }
        else
        {
          if (pos._node->_lnode)
          {
            (pos._node->_parent)->_rnode = pos._node->_lnode;
            (pos._node->_lnode)->_parent = pos._node->_parent;
          }
          else
          {
            (pos._node->_parent)->_rnode = pos._node->_rnode;
            (pos._node->_rnode)->_parent = pos._node->_parent;
          }
        }
        delete pos._node;
      }
      else //no child
      {
        if ((pos._node->_parent)->_lnode == pos._node) (pos._node->_parent)->_lnode = 0;
        else (pos._node->_parent)->_rnode = 0;
        delete pos._node;
      }
    }
    return true;
  }

  bool erase(const T& x) //remove firstly encountered x; (find then erase iterator)
  {
    iterator it = find(x);
    if (it == end()) return false;
    erase(it);
    return true;
  }

  iterator find(const T& x)
  {
    BSTreeNode<T>* cur = _root;
    while (cur->_data != x && cur && cur != _tail)
    {
      if (x > cur->_data) cur = cur->_rnode;
      else cur = cur->_lnode;
    }
    return ((cur->_data == x)? iterator(cur): end());
  }

  void clear() // delete all nodes except for the dummy node
  {
    while (!empty()) pop_front();
  }

  void sort() const {} //dummy command

  void print() const //-v
  {

  }

private:
  //data member
  BSTreeNode<T>* _root;
  BSTreeNode<T>* _tail;

  void swap(BSTreeNode<T>*& x, BSTreeNode<T>*& y) const
  {
    T temp = x->_data;
    x->_data = y->_data;
    y->_data = temp;
  }
  
};

#endif // BST_H
