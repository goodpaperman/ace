// map.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Map_Manager.h" 
#include "ace/Hash_Map_Manager.h" 
#include "ace/Null_Mutex.h" 
#include "ace/Recursive_Thread_Mutex.h" 
#include "ace/String_Base.h" 

//#define USE_TEMPLATE_SPECIALIZATION
#define USE_HASH_TABLE
#define USE_STR_KEY


class KeyType
{
public:
#if !defined(USE_TEMPLATE_SPECIALIZATION)
  friend bool operator == (KeyType const&, KeyType const&); 
#endif 

  KeyType() : val_(0) { }
  KeyType(int i) : val_(i) { }
  KeyType(KeyType const& kt) : val_(kt.val_) { }
  operator int() { return val_; } 
private:
  int val_; 
}; 


#ifdef USE_HASH_TABLE
#  ifdef USE_STR_KEY
typedef ACE_Hash_Map_Manager<ACE_TString, KeyType, ACE_Recursive_Thread_Mutex> MAP_TYPE; 
typedef ACE_Hash_Map_Bucket_Iterator<ACE_TString, KeyType, ACE_Hash<ACE_TString>, ACE_Equal_To<ACE_TString>, ACE_Recursive_Thread_Mutex> BUCKET_ITER; 
#  else
typedef ACE_Hash_Map_Manager<int, KeyType, ACE_Recursive_Thread_Mutex> MAP_TYPE; 
typedef ACE_Hash_Map_Bucket_Iterator<int, KeyType, ACE_Hash<int>, ACE_Equal_To<int>, ACE_Recursive_Thread_Mutex> BUCKET_ITER; 
#  endif 
#else 
//typedef ACE_Map_Manager<int, KeyType, ACE_Null_Mutex> MAP_TYPE; 
typedef ACE_Map_Manager<int, KeyType, ACE_Recursive_Thread_Mutex> MAP_TYPE; 
#endif 

#if defined(USE_TEMPLATE_SPECIALIZATION)
ACE_TEMPLATE_SPECIALIZATION
int MAP_TYPE::equal(int const& a, int const& b)
{
  return a == b; 
}
#endif 

class Map_Example
{
public:
  Map_Example() : map_(30) { }
  int run(); 
  int iterate_forward(); 
  int iterate_backward(); 
  int remove_all(); 
#ifdef USE_HASH_TABLE
  int iterate_bucket(); 
#endif 

private:
  MAP_TYPE map_; 
}; 


int Map_Example::run()
{
  ACE_TRACE(ACE_TEXT("Map_Example::run")); 

  //ACE_Recursive_Thread_Mutex &rtm(map_.mutex()); 
  ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, guard, map_.mutex(), -1);  
  for(int i=0; i<100; ++ i)
  {
#ifdef USE_STR_KEY
    ACE_TString str(80u); 
    sprintf((char*)str.c_str(), "%d", i); 
    str = str.c_str(); 
    if(map_.bind(str, KeyType(i)) != 0)
      ACE_DEBUG((LM_DEBUG, "bind %s with %d failed.\n", str.c_str(), i)); 
#else 
    map_.bind(i, KeyType(i)); 
#endif 
  }

  KeyType target; 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Map has \n"))); 
  for(int j=0; j<100; ++ j)
  {
#ifdef USE_STR_KEY
    ACE_TString str(80u); 
    sprintf((char*)str.c_str(), "%d", j); 
    str = str.c_str(); 
    if(map_.find(str, target) == 0)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d\n"), (int)target)); 
#else 
    if(map_.find(j, target) == 0)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d\n"), (int)target)); 
#endif
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  this->iterate_forward(); 
  this->iterate_backward();
#ifdef USE_HASH_TABLE
  this->iterate_bucket(); 
#endif  
  this->remove_all(); 
  this->iterate_forward(); 

  return 0; 
}

int Map_Example::iterate_forward()
{
  ACE_TRACE(ACE_TEXT("Map_Example::iterate_forward")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Forward iteration\n"))); 
  for(MAP_TYPE::iterator iter = map_.begin(); 
    iter != map_.end(); ++ iter)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d\n"), (int)((*iter).int_id_))); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

int Map_Example::iterate_backward()
{
  ACE_TRACE(ACE_TEXT("Map_Example::iterate_backward")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Backward iteration\n"))); 
  for(MAP_TYPE::reverse_iterator iter = map_.rbegin(); 
    iter != map_.rend(); ++ iter)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d\n"), (int)((*iter).int_id_))); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

int Map_Example::remove_all()
{
  map_.unbind_all(); 
  //
  //ACE_TRACE(ACE_TEXT("Map_Example::iterate_forward")); 
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("Forward iteration\n"))); 
  //for(MAP_TYPE::iterator iter = map_.begin(); 
  //  iter != map_.end(); ++ iter)
  //{
  //  ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d\n"), (int)((*iter).int_id_))); 
  //  map_.unbind((*iter).ext_id_); 
  //}

  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
  return 0; 
}

#ifdef USE_HASH_TABLE
int Map_Example::iterate_bucket()
{
  ACE_TRACE(ACE_TEXT("Map_Example::iterate_bucket")); 
  for(int i=0; i<map_.total_size(); ++ i)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("bucket %d iteration\n"), i)); 
#ifdef USE_STR_KEY
    ACE_TString str(80u); 
    sprintf((char*)str.c_str(), "%d", i); 
    str = str.c_str(); 
    
    BUCKET_ITER end(map_, str, 1); 
    for(BUCKET_ITER iter(map_, str); 
      iter != end; ++ iter)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d, "), (int)((*iter).int_id_))); 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
#else 
    BUCKET_ITER end(map_, i, 1); 
    for(BUCKET_ITER iter(map_, i); 
      iter != end; ++ iter)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("#%d, "), (int)((*iter).int_id_))); 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
#endif 
  }

  return 0; 
}
#endif 



int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  Map_Example me; 
  me.run(); 
	return 0;
}

