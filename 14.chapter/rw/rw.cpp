// rw.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <map> 
#include <string> 
using std::map; 
using std::string; 

#include "ace/Task.h" 
#include "ace/RW_Thread_Mutex.h" 
#include "ace/Guard_T.h" 
#include "ace/OS_NS_unistd.h" 

//#define USE_PROC

#if defined(USE_PROC)
typedef ACE_RW_Mutex RW_MUTEX; 
#else 
typedef ACE_RW_Thread_Mutex RW_MUTEX; 
#endif 

class HA_DiscoveryAgent
{
public:
  HA_DiscoveryAgent() 
    : device_map()
#if defined(USE_PROC)
    , rwmutex_(USYNC_PROCESS)
#endif 
  {
  }

  void add_device(string const name, string desc)
  {
    ACE_WRITE_GUARD(RW_MUTEX, mon, rwmutex_); 
    //ACE_READ_GUARD(RW_MUTEX, mon, rwmutex_); 
    device_map.insert(std::make_pair(name, desc)); 

    // to see the output more clearly.
    //ACE_OS::sleep(1); 
  }

  void remove_device(string const name)
  {
    ACE_WRITE_GUARD(RW_MUTEX, mon, rwmutex_); 
    device_map.erase(name); 
  }

  string const query_device(string const name)
  {
    ACE_READ_GUARD_RETURN(RW_MUTEX, mon, rwmutex_, ""); 
    map<string const, string>::iterator it = device_map.find(name); 
    if(it != device_map.end())
      return it->second; 
    else 
      return ""; 
  }

private:
  map<string const, string> device_map; 
  RW_MUTEX rwmutex_; 
};

HA_DiscoveryAgent agent; 

class QueryTask : public ACE_Task_Base
{
public:
  virtual int svc()
  {
    int n = 0; 
    char name[20] = { 0 }; 
    ACE_DEBUG((LM_DEBUG, "(%t) query thread start up.\n")); 
    while(ACE_Thread_Manager::instance()->testcancel(ACE_Thread::self()) == 0)
    {
      ACE_OS::sprintf(name, "%u", ++n); 
      string desc = agent.query_device(name); 
      if(desc != "")
      {
        ACE_DEBUG((LM_DEBUG, "(%t) %s: %s\n", name, desc.c_str())); 

        if(n >= 100)
          n = 0; 
      }
      else 
      {
        ACE_DEBUG((LM_DEBUG, "(%t) %s: not find\n", name)); 
        n = 0; 
      }

      ACE_OS::sleep(ACE_Time_Value(0, 200000)); 
    }

    return 0; 
  }
}; 

class ModifyTask : public ACE_Task_Base
{
public:
  virtual int svc()
  {
    int n = 0; 
    char name[20] = { 0 }; 
    char desc[60] = { 0 }; 
    ACE_DEBUG((LM_DEBUG, "(%t) modify thread start up.\n")); 
    ACE_OS::srand(ACE_OS::gettimeofday().sec()); 
    while(ACE_Thread_Manager::instance()->testcancel(ACE_Thread::self()) == 0)
    {
      ACE_OS::sprintf(name, "%u", ++n); 
      for(int i=0; i<60-1; ++ i)
        desc[i] = "abcdefghijklmnopqrstuvwxyz"[ACE_OS::rand()%26]; 

      agent.add_device(name, desc); 
      ACE_DEBUG((LM_DEBUG, "(%t) add %s: %s\n", name, desc)); 

      if(n >= 100)
        n = 0; 

      ACE_OS::sleep(ACE_Time_Value(0, 500000)); 
    }

    return 0; 
  }
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ModifyTask modify_task; 
  modify_task.activate(THR_NEW_LWP|THR_JOINABLE, 1); 

  QueryTask query_task; 
  query_task.activate(THR_NEW_LWP|THR_JOINABLE, 5); 

  ACE_OS::sleep(10); 
  ACE_Thread_Manager::instance()->cancel_all(); 

  modify_task.wait(); 
  query_task.wait(); 
	return 0;
}

