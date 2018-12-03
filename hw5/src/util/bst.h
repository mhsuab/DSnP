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
  class iterator{
    friend class BSTree;

  public:
    iterator(BSTreeNode<T>* n= 0): _node(n) {}
    iterator(const iterator& i) : _node(i._node) {}
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators
    const T& operator * () const { return _node->_data; }
    T& operator * () { return _node->_data; }
    iterator& operator ++ () {
      assert(_node->_lnode != _node);
      if (_node->_rnode) {
        _node = _node->_rnode;
        if (_node->_lnode != _node) { while (_node->_lnode) _node = _node -> _lnode; }
        return (*this);
      }
      else {
        while (_node->_parent && _node == (_node->_parent)->_rnode) _node = _node->_parent;
        _node = _node->_parent;
        return (*this);
      }
    }
    iterator operator ++ (int) {
      iterator temp = (*this);
      ++(*this);
      return temp;
    }
    iterator& operator -- () {
      if (_node->_lnode == _node) {
        _node = _node->_parent;
        return (*this);
      }
      else if (_node->_lnode) {
        _node = _node->_lnode;
        if (_node->_lnode != _node) { while (_node->_rnode) _node = _node->_rnode; }
        return (*this);
      }
      else {
        while (_node->_parent && _node == (_node->_parent)->_lnode) _node = _node->_parent;
        _node = _node->_parent;
        return (*this);
      }
    }
    iterator operator -- (int) {
      iterator temp = (*this);
      --(*this);
      return temp;
    }

    iterator& operator = (const iterator& i) {
      _node = i._node;
      return *(this);
    }

    bool operator != (const iterator& i) const { return (_node != i._node); }
    bool operator == (const iterator& i) const { return (_node == i._node); }
  private:
    BSTreeNode<T>* _node;

  };
  //leftmost element, end() if empty()
  iterator begin() const { return iterator(_tail->_rnode); }
  //handle it youself(but '--' bring back last element)
  iterator end() const { return iterator(_tail); }
  bool empty() const { return (_root == _tail); }
  size_t size() const {
    size_t n = 0;
    for (iterator it = begin(); it != end(); ++it) ++n;
    return n;
  }

  void insert(const T& x) {
    if (empty()) {
      _root = new BSTreeNode<T>(x, 0, _tail, 0);
      _tail->_rnode = _tail->_parent = _root;
    }
    else if (_root->_rnode->_rnode == _root) {
      // size == 1
      if (x > _root->_data) {
        BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, _tail, _root);
        _root->_rnode = b;
        _tail->_parent = b;
      }
      else {
        BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, 0, _root);
        _root->_lnode = b;
        _tail->_rnode = b;
      }
    }
    else {
      // size != 1
      BSTreeNode<T>* cur = _root, *prev = cur;
      while (cur && cur->_lnode != cur) {
        prev = cur;
        if (x > cur->_data) cur = cur->_rnode;
        else cur = cur->_lnode;
      }
      if (!cur) {
        if (_tail->_rnode == prev && x <= prev->_data) {
          // leftmost
          BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, 0, prev);
          prev->_lnode = b;
          _tail->_rnode = b;
        }
        else {
          // normal
          BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, 0, prev);
          if (x > prev->_data) prev->_rnode = b;
          else prev->_lnode = b;
        }
      }
      else {
        // rightmost
        BSTreeNode<T>* b = new BSTreeNode<T>(x, 0, _tail, prev);
        prev->_rnode = b;
        _tail->_parent = b;
      }
    }
  }
  //remove left most node
  void pop_front() {
    if (empty()) return;
    else if (_root->_rnode->_rnode == _root) {
      // size == 1
      delete _root;
      _tail->_parent = _tail->_rnode = _tail->_lnode = _tail;
      _root = _tail;
    }
    else {
      //size != 1
      if (_tail->_rnode == _root) {
        // leftmost == _root
        BSTreeNode<T>* leftmost = (++begin())._node;
        _tail->_rnode = leftmost;
        _root = _root->_rnode;
        delete _root->_parent;
        _root->_parent = 0;
      }
      else if ((_tail->_rnode)->_rnode) {
        // exist rightnode
        BSTreeNode<T>* leftmost = (++begin())._node;
        (_tail->_rnode)->_parent->_lnode = (_tail->_rnode)->_rnode;
        (_tail->_rnode)->_rnode->_parent = (_tail->_rnode)->_parent;
        delete _tail->_rnode;
        _tail->_rnode = leftmost;
      }
      else {
        //no rightnode
        _tail->_rnode = (_tail->_rnode)->_parent;
        delete (_tail->_rnode)->_lnode;
        (_tail->_rnode)->_lnode = 0;
      }
    }
  }
  //remove right most node
  void pop_back() {
    if (empty()) return;
    else if (_root->_rnode->_rnode == _root) {
      //size == 1
      pop_front();
    }
    else {
      // size != 1
      if (_tail->_parent == _root) {
        // rightmost == _root
        BSTreeNode<T>* rightmost = (--(--end()))._node;
        _tail->_parent = rightmost;
        _root = _root->_lnode;
        delete _root->_parent;
        _root->_parent = 0;
        rightmost->_rnode = _tail;
      }
      else if ((_tail->_parent)->_lnode) {
        // exist leftnode
        BSTreeNode<T>* rightmost = (--(--end()))._node;
        (_tail->_parent)->_parent->_rnode = (_tail->_parent)->_lnode;
        (_tail->_parent)->_lnode->_parent = (_tail->_parent)->_parent;
        delete _tail->_parent;
        _tail->_parent = rightmost;
        rightmost->_rnode = _tail;
      }
      else {
        //no leftnode
        _tail->_parent = (_tail->_parent)->_parent;
        delete (_tail->_parent)->_rnode;
        (_tail->_parent)->_rnode = _tail;
      }
    }
  }

  // return false if nothing to erase
  bool erase(iterator pos) {
    if (empty()) return false;
    else {
      if (pos == begin()) { pop_front(); return true; }
      else if (pos == (--end())) { pop_back(); return true; }
      if (pos._node->_lnode && pos._node->_rnode) {
        // two children
        BSTreeNode<T>* cur = pos._node;
        ++pos;
        swap(cur, pos._node);
      }
      if (pos._node->_lnode || pos._node->_rnode) {
        // one child
        if ((pos._node->_parent)->_lnode == pos._node) {
          // left child
          if (pos._node->_lnode) {
            // exist left child
            (pos._node->_parent)->_lnode = pos._node->_lnode;
            (pos._node->_lnode)->_parent = pos._node->_parent;
          }
          else {
            // exist right child
            (pos._node->_parent)->_lnode = pos._node->_rnode;
            (pos._node->_rnode)->_parent = pos._node->_parent;
          }
        }
        else {
          // right child
          if (pos._node->_lnode) {
            //exist left child
            (pos._node->_parent)->_rnode = pos._node->_lnode;
            (pos._node->_lnode)->_parent = pos._node->_parent;
          }
          else {
            (pos._node->_parent)->_rnode = pos._node->_rnode;
            (pos._node->_rnode)->_parent = pos._node->_parent;
          }
        }
        delete pos._node;
      }
      else {
        // no child
        if (pos._node == (pos._node->_parent)->_lnode) (pos._node->_parent)->_lnode = 0;
        else (pos._node->_parent)->_rnode = 0;
        delete pos._node;
      }
      return true; 
    }
  }

  //remove firstly encountered x; (find then erase iterator)
  bool erase(const T& x) {
    iterator it = find(x);
    if (it == end()) return false;
    erase(it);
    return true;
  }

  iterator find(const T& x) {
    BSTreeNode<T>* cur = _root;
    while (cur && cur->_data != x && cur != _tail) {
      if (x > cur->_data) cur = cur->_rnode;
      else cur = cur->_lnode;
    }
    return (!cur || cur == _tail)? end(): iterator(cur);
  }

  // delete all nodes except for the dummy node
  void clear() { while (!empty()) pop_front(); }

  void sort() const {} //dummy command

  void print() const { printv(_root, 0); }

private:
  //data member
  BSTreeNode<T>* _root;
  BSTreeNode<T>* _tail;

  void swap(BSTreeNode<T>*& x, BSTreeNode<T>*& y) const {
    T temp = x->_data;
    x->_data = y->_data;
    y->_data = temp;
  }

  void printv(BSTreeNode<T>* r, size_t n) const {
    if (!r) return;
    cout << string(n, ' ') << r->_data << endl;
    n += 2;
    if (r->_lnode) printv(r->_lnode, n);
    else cout << string(n, ' ') << "[0]" << endl;
    if (r->_rnode && r->_rnode != _tail) printv(r->_rnode, n);
    else cout << string(n, ' ') << "[0]" << endl;
  }
  
};

#endif // BST_H
