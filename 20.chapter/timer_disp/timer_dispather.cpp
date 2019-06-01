#include "StdAfx.h"
#include "timer_dispather.h"

#define USE_HEAP_IMPL
//#define USE_LIST_IMPL
//#define USE_WHEEL_IMPL
//#define USE_HASH_HEAP_IMPL
//#define USE_HASH_LIST_IMPL
 
#if defined(USE_HEAP_IMPL)
#include "ace/Timer_Heap.h"
#elif defined(USE_LIST_IMPL)
#include "ace/Timer_List.h" 
#elif defined(USE_WHEEL_IMPL)
#include "ace/Timer_Wheel.h" 
#elif defined(USE_HASH_HEAP_IMPL) || defined(USE_HASH_LIST_IMPL)
#include "ace/Timer_Hash.h" 
#endif 


timer_dispatcher::timer_dispatcher(void)
: timer_queue_(0)
, timer_()
{
#if defined(USE_HEAP_IMPL)
  ACE_NEW(timer_queue_, ACE_Timer_Heap); 
#elif defined(USE_LIST_IMPL)
  ACE_NEW(timer_queue_, ACE_Timer_List); 
#elif defined(USE_WHEEL_IMPL)
  ACE_NEW(timer_queue_, ACE_Timer_Wheel); 
#elif defined(USE_HASH_HEAP_IMPL)
  ACE_NEW(timer_queue_, ACE_Timer_Hash_Heap); 
#elif defined(USE_HASH_LIST_IMPL)
  ACE_NEW(timer_queue_, ACE_Timer_Hash); 
#else
  //ACE_NOTSUP_RETURN(-1); 
#endif 
}

timer_dispatcher::~timer_dispatcher(void)
{
  delete timer_queue_; 
  timer_queue_ = 0; 
}

void timer_dispatcher::wait_for_event(int *running)
{
  ACE_TRACE(ACE_TEXT("timer_dispatcher::wait_for_event")); 
  while(*running)
  {
    ACE_Time_Value max(1, 0); //timer_queue_->gettimeofday(); 
    ACE_Time_Value *timeout = timer_queue_->calculate_timeout(&max); 
    if(*timeout == ACE_Time_Value::zero)
      timer_queue_->expire(); 
    else 
    {
      ACE_Time_Value tv = timer_queue_->gettimeofday(); 
      tv += *timeout; 
      timer_.wait(&tv); 
      timer_queue_->expire(); 
    }
  }
}

long timer_dispatcher::schedule(ACE_Event_Handler* cb, void* arg, 
                               ACE_Time_Value const& abstime, 
                               ACE_Time_Value const& interval /* =ACE_Time_Value::zero */)
{
  ACE_TRACE(ACE_TEXT("timer_dispatcher::schedule")); 
  return timer_queue_->schedule(cb, arg, abstime, interval); 
}

int timer_dispatcher::cancel(long timer_id, void const** arg /* = 0 */, int dont_call_handle_close /* = 1 */)
{
  ACE_TRACE(ACE_TEXT("timer_dispatcher::cancel")); 
  return timer_queue_->cancel(timer_id, arg, dont_call_handle_close); 
}

int timer_dispatcher::reset_interval(long timer_id, ACE_Time_Value const& interval) 
{
  ACE_TRACE(ACE_TEXT("timer_dispatcher::reset_interval")); 
  return timer_queue_->reset_interval(timer_id, interval); 
}