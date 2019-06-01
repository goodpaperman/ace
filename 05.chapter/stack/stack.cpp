// stack.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Containers.h" 
#include "ace/Unbounded_Queue.h" 

#include <set>
#include <deque> 
#include <vector> 

using std::set; 
using std::deque; 
using std::vector; 

void testContainerSize()
{
  set<int> iset; 
  deque<int> ique; 
  vector<int> ivec; 
  set<int>::iterator set_iter = iset.begin(); 
  deque<int>::iterator que_iter = ique.begin(); 
  vector<int>::iterator vec_iter = ivec.begin(); 

  printf("sizeof(iset) = %d.\n", sizeof(iset)); 
  printf("sizeof(ique) = %d.\n", sizeof(ique)); 
  printf("sizeof(ivec) = %d.\n", sizeof(ivec)); 
  printf("sizeof(set_iter) = %d.\n", sizeof(set_iter)); 
  printf("sizeof(que_iter) = %d.\n", sizeof(que_iter)); 
  printf("sizeof(vec_iter) = %d.\n", sizeof(vec_iter)); 
  printf("\n"); 

  int const size = 10; 
  int arr[size] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }; 
  iset.insert(arr, arr+size); 
  ique.insert(ique.begin(), arr, arr+size); 
  ivec.insert(ivec.begin(), arr, arr+size); 

  printf("sizeof(iset) = %d.\n", sizeof(iset)); 
  printf("sizeof(ique) = %d.\n", sizeof(ique)); 
  printf("sizeof(ivec) = %d.\n", sizeof(ivec)); 

  for(vec_iter = ivec.begin(); vec_iter != ivec.end(); ++ vec_iter)
  {
    printf("%d\n", *vec_iter); 
  }
}


class DataElement
{
public:
  DataElement() { ++ count_; } 
  ~DataElement() { -- count_; } 
  DataElement(int data) : data_(data) { ++ count_; } 
  DataElement(DataElement const& e) { data_ = e.getData(); ++ count_; } 
  DataElement& operator= (DataElement const& e) { data_ = e.getData(); return *this; } 
  bool operator== (DataElement const& e) const { return this->data_ == e.data_; } 
  bool operator!= (DataElement const& e) const { return !(*this == e); } 

  int getData() const { return data_; } 
  void setData(int val) { data_ = val; } 

  static int numOfActiveOjbects() { return count_; } 

private:
  int data_; 
  static int count_; 
}; 

int DataElement::count_ = 0; 

int runBoundedStack()
{
  ACE_TRACE(ACE_TEXT("runBoundedStatck")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using a bounded stack\n"))); 
  ACE_Bounded_Stack<DataElement> bstack1(100); 
  
  {
    DataElement elem[100]; 
    for(int i=0; i<100; ++ i)
    {
      elem[i].setData(i); 
      bstack1.push(elem[i]); 
    }
  }

  ACE_Bounded_Stack<DataElement> bstack2(100); 
  bstack2 = bstack1; 
  for(int j=0; j<bstack2.size(); ++ j)
  {
    DataElement elem; 
    bstack2.pop(elem); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem.getData())); 
  }

  return 0; 
}

int runFixedStack()
{
  ACE_TRACE(ACE_TEXT("runFixedStack")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using a fixed stack\n"))); 

  //DataElement* elem; 
  DataElement elem; 
  ACE_Fixed_Stack<DataElement, 100> fstack; 
  for(int k=0; k<100; ++k)
  {
    //ACE_NEW_RETURN(elem, DataElement(k), -1); 
    //fstack.push(elem); 
    fstack.push(DataElement(k)); 
  }

  for(int l=0; l<100; ++ l)
  {
    fstack.pop(elem); 
    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem->getData())); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem.getData())); 
    //delete elem; 
  }

  return 0; 
}

int runUnboundedStack()
{
  ACE_TRACE(ACE_TEXT("runUnboundedStack")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using an unbounded stack\n"))); 
  //ACE_Unbounded_Stack<DataElement*> ustack; 
  ACE_Unbounded_Stack<DataElement> ustack; 
  for(int m=0; m<100; ++m)
  {
    //DataElement* elem; 
    //ACE_NEW_RETURN(elem, DataElement(m), -1); 
    //ustack.push(elem); 
    ustack.push(DataElement(m)); 
  }

  //ACE_Unbounded_Stack_Iterator<DataElement*> iter(ustack); 
  ACE_Unbounded_Stack_Iterator<DataElement> iter(ustack); 
  for(iter.first(); !iter.done(); iter.advance())
  {
    //DataElement** elem; 
    DataElement *elem ; 
    iter.next(elem); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem->getData())); 
    delete elem; 

    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), (*elem)->getData())); 
    //delete (*elem); 
  }

  return 0; 
}

int runUnboundedQueue()
{
  ACE_TRACE(ACE_TEXT("runUnboundedQueue")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using an unbounded queue\n"))); 
  ACE_Unbounded_Queue<DataElement> queue; 
  for(int i=0; i<10; ++ i)
  {
    queue.enqueue_head(DataElement(9-i)); 
  }

  for(int i=0; i<10; ++ i)
    queue.enqueue_tail(DataElement(10+i)); 

  DataElement* elem = 0; 
  for(ACE_Unbounded_Queue_Const_Iterator<DataElement> iter(queue); 
    !iter.done(); iter.advance())
  {
    iter.next(elem); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem->getData())); 
  }

  return 0; 
}

int runHeapUnboundedQueue()
{
  ACE_TRACE(ACE_TEXT("runHeapUnboundedQueue")); 
  ACE_Unbounded_Queue<DataElement*> queue; 
  DataElement* elem; 

  for(int i=0; i<20; ++ i)
  {
    ACE_NEW_RETURN(elem, DataElement(i), -1); 
    queue.enqueue_tail(elem); 
  }

  while(queue.dequeue_head(elem) != -1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), elem->getData())); 
    delete elem; 
  }

  return 0; 
}

int runArray()
{
  ACE_Array<DataElement> arr(10); 
  for(int i=0; i<10; ++ i)
    arr[i] = DataElement(i); 

  DataElement elem; 
  ACE_ASSERT(arr.get(elem, 11) == -1); 
  ACE_ASSERT(arr.set(elem, 11) == -1); 

  ACE_Array<DataElement> copy = arr; 
  ACE_ASSERT(copy == arr); 

  DataElement* ptr= 0; 
  ACE_Array<DataElement>::ITERATOR iter(arr); 
  while(!iter.done())
  {
    iter.next(ptr); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d\n"), ptr->getData())); 
    iter.advance(); 
  }

  return 0; 
}

int runBoundedSet()
{
  ACE_TRACE(ACE_TEXT("runBoundedSet")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using a bounded set\n"))); 
  ACE_Bounded_Set<DataElement> bset(100); 
  for(int i=0; i<100; ++ i)
  {
    bset.insert(DataElement(i)); 
    if(bset.insert(DataElement(99-i)) == -1)
      ACE_DEBUG((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("insert set"))); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("active objects=%d.\n"), DataElement::numOfActiveOjbects())); 
  DataElement e1(10), e2(99); 
  if(bset.find(e1) == 0 && bset.find(e2) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("The elements %d and %d are in the set!\n"), e1.getData(), e2.getData())); 

  for(int i=0; i<50; ++ i)
  {
    bset.remove(DataElement(i)); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove %d\n"), i)); 
  }

  if(bset.find(DataElement(0)) == -1 && bset.find(DataElement(49)) == -1)
  {
    ACE_DEBUG((LM_INFO, ACE_TEXT("The elements %d and %d are NOT in the set!\n"), 0, 49)); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("active objects=%d.\n"), DataElement::numOfActiveOjbects())); 
  return 0; 
}

int runUnboundedSet()
{
  ACE_TRACE(ACE_TEXT("runUnboundedSet")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Using an unbounded set.\n"))); 
  ACE_Unbounded_Set<DataElement*> uset; 
  DataElement* elem = 0; 
  for(int m=0; m<100; ++ m)
  {
    ACE_NEW_RETURN(elem, DataElement(m), -1); 
    uset.insert(elem); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("active objects=%d.\n"), DataElement::numOfActiveOjbects())); 

  {
    DataElement beg(0), end(99); 
    if(uset.find(&beg) == 0 && uset.find(&end) == 0)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("Found the elements\n"))); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Deleting the elements\n"))); 
  ACE_Unbounded_Set_Iterator<DataElement*> iter(uset); 
  for(iter = uset.begin(); iter != uset.end(); ++iter)
  {
    elem = *iter; 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d.\n"), elem->getData())); 
    delete elem; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("active objects=%d.\n"), DataElement::numOfActiveOjbects())); 
  return 0;
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //testContainerSize(); 
  //runBoundedStack(); 
  //runFixedStack();
  //runUnboundedStack(); 
  //runUnboundedQueue(); 
  //runHeapUnboundedQueue(); 
  //runArray(); 
  //runBoundedSet(); 
  runUnboundedSet(); 
	return 0;
}

