// condition.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Thread_Mutex.h" 
#include "ace/Condition_T.h" 
#include "ace/Guard_T.h" 
#include "ace/os_ns_unistd.h" 
#include "ace/Task_T.h" 
#include "ace/Message_Queue.h" 

class HA_Device_Repository
{
public:
  HA_Device_Repository() : owner_(0)
  {
  }

  int is_free()
  {
    return this->owner_ == 0; 
  }

  int is_owner(ACE_Task_Base* tb)
  {
    return this->owner_ == tb; 
  }

  ACE_Task_Base* get_owner()
  {
    return this->owner_; 
  }

  void set_owner(ACE_Task_Base* tb)
  {
    this->owner_ = tb; 
  }

  int update_device(int device_id)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Updating Device %d\n"), device_id)); 
    ACE_OS::sleep(1); 
    return 0; 
  }

private:
  ACE_Task_Base *owner_; 
}; 

class HA_CommandHandler : public ACE_Task_Base
{
public:
  enum { NUM_USES = 10 }; 
  HA_CommandHandler(HA_Device_Repository &rep, ACE_Thread_Mutex &mutex, ACE_Condition<ACE_Thread_Mutex> &cond)
    : rep_(rep)
    , mutex_(mutex)
    , cond_(cond)
  {
  }

  virtual int svc()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Handler Thread running\n"))); 
    for(int i=0; i<NUM_USES; ++ i)
    {
      {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, this->mutex_, -1); 
        while(!this->rep_.is_free())
          this->cond_.wait(); 

        this->rep_.set_owner(this); 
      }

      this->rep_.update_device(i); 
      ACE_ASSERT(this->rep_.is_owner(this)); 
      this->rep_.set_owner(0); 

      {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, this->mutex_, -1); 
        this->cond_.signal(); 
        ACE_OS::sleep(ACE_Time_Value(0, 100000)); 
        //ACE_Thread::yield(); 
      }
    }

    return 0; 
  }

private:
  HA_Device_Repository &rep_; 
  ACE_Thread_Mutex &mutex_; 
  ACE_Condition<ACE_Thread_Mutex> &cond_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_DEBUG((LM_DEBUG, 
    ACE_TEXT("sizeof(ACE_Task_Base) = %d\n")
    ACE_TEXT("sizeof(ACE_Task) = %d\n")
    ACE_TEXT("sizeof(ACE_Message_Queue) = %d\n"), 
    sizeof(ACE_Task_Base), 
    sizeof(ACE_Task<ACE_MT_SYNCH>), 
    sizeof(ACE_Message_Queue<ACE_MT_SYNCH>))); 

  HA_Device_Repository rep; 
  ACE_Thread_Mutex mutex; 
  ACE_Condition<ACE_Thread_Mutex> cond(mutex); 

  HA_CommandHandler handler1(rep, mutex, cond); 
  HA_CommandHandler handler2(rep, mutex, cond); 

  handler1.activate(); 
  handler2.activate(); 

  handler1.wait(); 
  handler2.wait(); 
	return 0;
}

