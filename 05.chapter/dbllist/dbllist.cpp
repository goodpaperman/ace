// dbllist.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "ace/Containers_T.h" 
#include "ace/Containers.h" 


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

typedef ACE_DLList<DataElement> MyList; 


class ListTest
{
public:
  int run(); 
  void displayList(MyList& list); 
  void destroyList(MyList& list); 
}; 

int ListTest::run()
{
  ACE_TRACE(ACE_TEXT("ListTest::run")); 
  MyList list1; 
  for(int i=0; i<10; ++ i)
  {
    DataElement* element; 
    ACE_NEW_RETURN(element, DataElement(i), -1); 
    list1.insert_tail(element); 
  }

  this->displayList(list1); 

  MyList list2; 
  list2 = list1; 
  this->displayList(list2); 
  this->destroyList(list2); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("# of live objects: %d\n"), DataElement::numOfActiveOjbects())); 
  return 0; 
}

void ListTest::displayList(MyList& list)
{
  ACE_TRACE(ACE_TEXT("ListTest::displayList")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Forward iteration\n"))); 
  ACE_DLList_Iterator<DataElement> iter(list); 
  while(!iter.done())
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), iter.next()->getData())); 
    iter++; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\nReverse Iteration\n"))); 
  ACE_DLList_Reverse_Iterator<DataElement> riter(list); 
  while(!riter.done())
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), riter.next()->getData())); 
    riter++; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
}

void ListTest::destroyList(MyList& list)
{
  //list.reset(); //delete_nodes(); 
  for(ACE_DLList_Iterator<DataElement> iter(list); !iter.done(); ++ iter)
  {
    DataElement* de = iter.next(); 
    delete de; 
  }
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ListTest lt; 
  lt.run(); 
	return 0;
}

