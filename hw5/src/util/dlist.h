/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
  friend class DList<T>;
  friend class DList<T>::iterator;

  DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
    _data(d), _prev(p), _next(n) {}
  
  // [NOTE] DO NOT ADD or REMOVE any data member
  T              _data;
  DListNode<T>*  _prev;
  DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
  // TODO: decide the initial value for _isSorted
  DList() {
    _head = new DListNode<T>(T());
    _head->_prev = _head->_next = _head; // _head is a dummy node
  }
  ~DList() { clear(); delete _head; }

  // DO NOT add any more data member or function for class iterator
  class iterator
  {
    friend class DList;
  public:
    iterator(DListNode<T>* n= 0): _node(n) {}
    iterator(const iterator& i) : _node(i._node) {}
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators
    const T& operator * () const { return _node->_data; }
    T& operator * () { return _node->_data; }
    iterator& operator ++ ()
    {
      _node = _node->_next;
      return *(this);
    }
    iterator operator ++ (int)
    {
      iterator temp = (*this);
      ++(*this);
      return temp;
    }
    iterator& operator -- ()
    {
      _node = _node->_prev;
      return *(this);
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
    DListNode<T>* _node;
  };

  // TODO: implement these functions
  iterator begin() const { return iterator(_head->_next); }
  iterator end() const { return iterator(_head); }
  bool empty() const { return (_head->_next == _head); }
  size_t size() const {
    size_t n = 0;
    for (iterator it = begin(); it != end(); ++it) ++n;
    return n;
  }

  void push_back(const T& x) {
    DListNode<T>* add = new DListNode<T>(x, _head->_prev, _head);
    (_head->_prev)->_next = add;
    _head->_prev = add;
    _isSorted = false;
  }
  void pop_front() {
    DListNode<T>* first = _head->_next;
    _head->_next = first->_next;
    (first->_next)->_prev = _head;
    delete first;
  }
  void pop_back() {
    DListNode<T>* last = _head->_prev;
    _head->_prev = last->_prev;
    (last->_prev)->_next = _head;
    delete last;
  }

  // return false if nothing to erase
  bool erase(iterator pos) {
    if (empty()) return false;
    ((pos._node)->_prev)->_next = (pos._node)->_next;
    ((pos._node)->_next)->_prev = (pos._node)->_prev;
    delete pos._node;
    return true;
  }
  bool erase(const T& x) {
    iterator it = find(x);
    if (it == end()) return false;
    else {
      erase(it);
      return true;
    }
  }

  iterator find(const T& x) {
    for (iterator it = begin(); it != end(); ++it) { if (*it == x) return it; }
    return end();
  }

  void clear() // delete all nodes except for the dummy node
  {
    while (!empty()) pop_front();
  }

  /*
  void sort() const //selection sort
  {
    if (empty() || _isSorted) return;
    else if (size() == 1)
    {
      _isSorted = true;
      return;
    }
    else
    {
      for (iterator i = begin(); i != end(); ++i)
      {
        T min = (i._node)->_data;
        iterator k = i;
        for (iterator j = i; j != end(); ++j)
        {
          if ((j._node)->_data < min)
          {
            min = (j._node)->_data;
            k = j;
          }
        }
        if (k != i) swap(i._node, k._node);
      }
      _isSorted = true;
    }
  }
  */

  void sort() const {
    if (!empty() && !_isSorted) quicksort(_head->_next, _head->_prev);
    _isSorted = true;
  }

private:
  // [NOTE] DO NOT ADD or REMOVE any data member
  DListNode<T>*  _head;     // = dummy node if list is empty
  mutable bool   _isSorted; // (optionally) to indicate the array is sorted

  // [OPTIONAL TODO] helper functions; called by public member functions
  void swap(DListNode<T>*& x, DListNode<T>*& y) const {
    T temp = x->_data;
    x->_data = y->_data;
    y->_data = temp;
  }

  void quicksort(iterator left, iterator right) const //segmentation fault: may be coming from ?._node
  {
    if ((left == right)) return;
    iterator i = left, j = left;
    T mid = (right._node)->_data;
    for (;i != right; ++i)
    {
      if ((i._node)->_data < mid)
      {
        swap(i._node, j._node);
        ++j;
      }
    }
    swap(j._node, right._node);
    if (j == left)
    {
      quicksort(++j, right);
    }
    else if (j == right)
    {
      quicksort(left, --j);
    }
    else
    {
      quicksort(left, iterator(j._node->_prev));
      quicksort(iterator(j._node->_next), right);
    }
  }
};

#endif // DLIST_H
