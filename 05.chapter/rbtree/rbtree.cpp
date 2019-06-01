// rbtree.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/RB_Tree.h" 
#include "ace/Null_Mutex.h" 

template <class EXT_ID, class INT_ID>
class Tree : public ACE_RB_Tree<EXT_ID, INT_ID, ACE_Less_Than<EXT_ID>, ACE_Null_Mutex>
{
public:

};

class DataElement
{
public:
  DataElement() { ++ count_; } 
  ~DataElement() { -- count_; } 
  DataElement(int data) : data_(data) { ++ count_; } 
  DataElement(DataElement const& e) { data_ = e.getData(); ++ count_; } 
  DataElement& operator= (DataElement const& e) { data_ = e.getData(); return *this; } 
  bool operator== (DataElement const& e) { return this->data_ == e.data_; } 

  int getData() const { return data_; } 
  void setData(int val) { data_ = val; } 

  static int numOfActiveOjbects() { return count_; } 

private:
  int data_; 
  static int count_; 
}; 

int DataElement::count_ = 0; 

#define USE_TEMPLATE_SPECIALIZATION

class KeyType
{
public:
  KeyType() : val_(0) { }
  KeyType(int i) : val_(i) { }
  KeyType(KeyType const& kt) : val_(kt.val_) { }

  operator int() const { return val_; } 
#if !defined(USE_TEMPLATE_SPECIALIZATION)
  bool operator <(KeyType const& rhs) const  { return this->val_ < rhs.val_; } 
#endif 

private:
  int val_; 
}; 

#if defined(USE_TEMPLATE_SPECIALIZATION)
ACE_TEMPLATE_SPECIALIZATION
class ACE_Less_Than<KeyType>
{
public:
  int operator() (KeyType const& k1, KeyType const& k2)
  { return (int)k1 < /*(int)*/k2; } 
}; 
#endif 


class Tree_Example
{
public:
  int run(); 
  int iterate_forward(); 
  int iterate_backward(); 
  int remove_all(); 

private:
  Tree<KeyType, DataElement> tree_; 
}; 

int Tree_Example::run()
{
  ACE_TRACE(ACE_TEXT("Tree_Example::run")); 
  for(int i=0; i<100; ++ i)
  {
    if(tree_.bind(KeyType(i), DataElement(100-i)) != 0)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("bind")), -1); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using find: \n"))); 
  DataElement data; 
  for(int j=0; j<100; ++ j)
    if(tree_.find(KeyType(j), data) == 0)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d, %d\n"), j, data.getData())); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  this->iterate_forward(); 
  this->iterate_backward();
  int ret = this->remove_all(); 
  ACE_ASSERT(ret != -1); 
  this->iterate_forward(); 
  return 0; 
}

int Tree_Example::iterate_forward()
{
  ACE_TRACE(ACE_TEXT("Tree_Example::iterate_forward")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("forward iteration:\n"))); 
  for(Tree<KeyType, DataElement>::iterator iter = tree_.begin(); 
    iter != tree_.end(); ++iter)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d, "), (*iter).item().getData())); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

int Tree_Example::iterate_backward()
{
  ACE_TRACE(ACE_TEXT("Tree_Example::iterate_backward")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("backward iteration:\n"))); 
  for(Tree<KeyType, DataElement>::reverse_iterator iter = tree_.rbegin(); 
    iter != tree_.rend(); ++iter)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d, "), (*iter).item().getData())); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

int Tree_Example::remove_all()
{
  ACE_TRACE(ACE_TEXT("Tree_Example::remove_all")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Removing elements\n"))); 
  //tree_.clear(); 

  DataElement data; 
  for(int i=0; i<100; ++ i)
  {
    if(tree_.unbind(KeyType(i), data) != 0)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("Unbind")), -1); 
    else 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d, "), data.getData())); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  Tree_Example te; 
  te.run(); 
	return 0;
}

