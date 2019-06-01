// mutex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Thread_Mutex.h" 
#include "ace/Guard_T.h" 
#include "ace/os_ns_unistd.h" 

//class Large_Object
//{
//public:
//  Large_Object()
//  {
//    ACE_NEW(mem_, char[1000000000]()); 
//  }
//
//  ~Large_Object()
//  {
//    delete[] mem_; 
//  }
//
//private:
//  char* mem_; 
//}; 

class HA_Device_Repository
{
public:
  HA_Device_Repository() { }
  int update_device(int device_id)
  {
    //mutex_.acquire(); 
    //ACE_Guard<ACE_Thread_Mutex> guard(mutex_); 
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, mutex_, -1); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Updating device %d\n"), device_id)); 
    ACE_OS::sleep(1); 

    //Large_Object *obj = 0; 
    //ACE_NEW_RETURN(obj, Large_Object, -1); 
    //delete obj; 

    if(device_id % 6 == 5)
      return -1; 

    //mutex_.release(); 
    return 0; 
  }

private:
  ACE_Thread_Mutex mutex_; 
}; 

class HA_CommandHandler : public ACE_Task_Base
{
public:
  enum { NUM_USES = 10 }; 
  HA_CommandHandler(HA_Device_Repository &rep) : rep_(rep)
  {
  }

  virtual int svc()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Handler Thread running\n"))); 
    for(int i=0; i<NUM_USES; ++ i)
      this->rep_.update_device(i); 

    return 0; 
  }

private:
  HA_Device_Repository& rep_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Main Thread running\n"))); 
  HA_Device_Repository rep; 
  HA_CommandHandler handler1(rep); 
  HA_CommandHandler handler2(rep); 

  int result = handler1.activate(); 
  ACE_ASSERT(result == 0); 
  result = handler2.activate(); 
  ACE_ASSERT(result == 0); 

  handler1.wait(); 
  handler2.wait(); 
	return 0;
}

