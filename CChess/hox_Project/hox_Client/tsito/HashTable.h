#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include	<iostream>

using namespace std;
/*
 * HashTable.h (c) Noah Roberts 2003-03-13
 * HashTable keeps a hash of board positions.
 */

#include	"Board.h"


template<class T>
class HashTable
{
  T		*content;
  unsigned int	mask;

 public:
  HashTable(unsigned int bitcount) // size is a count of bits, not actual size.
    {
      int size = 1 << bitcount;
      content = new T[size];
      mask = size - 1;
    }
  void insert(unsigned long key, T &nde)
    {
      content[key & mask] = nde;
    }
  T &find(unsigned long key)
    {
      return content[key & mask];
    }
};

class HashNode
{
 protected:
  unsigned int _key;
  unsigned int _lock;
 public:
  HashNode() { _key = 0; _lock = 0; } // so a hit never occurs on the default.
  HashNode(Board *board) { setKeys(board); }
  unsigned int key() { return _key; }
  unsigned int lock() { return _lock; }
  void setKeys(Board *board)
    {
      _key = board->primaryHash();
      _lock = board->secondaryHash();
    }
  bool operator==(HashNode &nde) { return (_key == nde._key && _lock == nde._lock); }

};


#endif /* __HASHTABLE_H__ */
