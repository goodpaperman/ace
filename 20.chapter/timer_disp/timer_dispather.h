#pragma once

#include "ace/Event.h" 
#include "ace/Event_Handler.h" 
#include "ace/Timer_Queue.h" 
#include "ace/Singleton.h" 
#include "ace/Null_Mutex.h" 

class timer_dispatcher
{
public:
  timer_dispatcher(void);
  virtual ~timer_dispatcher(void);

  void wait_for_event(int* running); 
  long schedule(ACE_Event_Handler* cb, void* arg, 
    ACE_Time_Value const& abstime, 
    ACE_Time_Value const& interval = ACE_Time_Value::zero); 

  int cancel(long timer_id, void const** arg = 0, int dont_call_handle_close = 1); 
  int reset_interval(long timer_id, ACE_Time_Value const& interval); 

private:
  ACE_Timer_Queue* timer_queue_; 
  ACE_Event timer_; 
};

typedef ACE_Singleton<timer_dispatcher, ACE_Null_Mutex> TIMER; 