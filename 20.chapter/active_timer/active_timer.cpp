// active_timer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Timer_Queue_Adapters.h" 
#include <algorithm> 

//#define USE_HEAP_IMPL
//#define USE_LIST_IMPL
//#define USE_WHEEL_IMPL
//#define USE_HASH_HEAP_IMPL
#define USE_HASH_LIST_IMPL
 
#if defined(USE_HEAP_IMPL)
#include "ace/Timer_Heap.h"
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Heap> ActiveTimer; 
#elif defined(USE_LIST_IMPL)
#include "ace/Timer_List.h" 
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_List> ActiveTimer; 
#elif defined(USE_WHEEL_IMPL)
#include "ace/Timer_Wheel.h" 
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Wheel> ActiveTimer; 
#elif defined(USE_HASH_HEAP_IMPL)
#include "ace/Timer_Hash.h" 
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Hash_Heap> ActiveTimer; 
#elif defined(USE_HASH_LIST_IMPL)
#include "ace/Timer_Hash.h" 
typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Hash> ActiveTimer; 
#endif 

int _running = 0; 


class timer_handler : public ACE_Event_Handler
{
public:
  timer_handler() : timer_id_(-1), interval_(0) { } 
  int timer_id() const { return timer_id_; } 
  void timer_id(int id) 
  { 
    timer_id_ = id; 
  } 

  int interval() const { return interval_; } 
  void interval(int val) 
  {
    interval_ = val;
    ACE_DEBUG((LM_DEBUG, "create timer handler %d = %d.\n", timer_id_, interval_));  
  }

  virtual int handle_timeout(ACE_Time_Value const& current_time, void const* act = 0)
  {
    ACE_DEBUG((LM_DEBUG, "(%t) timeout: %d.\n", timer_id_)); 
    return 0; 
  }

  virtual ~timer_handler()
  {
    ACE_DEBUG((LM_DEBUG, "close timer handler %d.\n", timer_id_)); 
  }

private:
  int timer_id_; 
  int interval_; 
}; 

int test_timer_queue(ActiveTimer* tq)
{
  int const size = 10; 
  int ids[size] = { 0 }; 
  int val[size] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }; 
  timer_handler *handler = 0; 
  ACE_Time_Value future_time[size];  
  ACE_Time_Value interval[size]; 

  srand(ACE_OS::thr_self()); 
  std::random_shuffle(val, val+size); 

  for(int i=0; i<size; ++ i)
  {
    future_time[i] = ACE_OS::gettimeofday() + (ACE_Time_Value)val[i]; 
    interval[i] = val[size-i-1]; 
  }

  for(int i=0; i<size; ++ i)
  {
    ACE_NEW_RETURN(handler, timer_handler, -1); 
    ids[i] = tq->schedule(handler, handler, future_time[i], interval[i]); 
    if(ids[i] == -1)
      return -1; 

    handler->timer_id(ids[i]); 
    handler->interval(interval[i].sec()); 
  }

  //ACE_Time_Value max_wait(1); 

  //while(_running)
  //{
  //  ACE_Time_Value* timeout = tq->calculate_timeout(&max_wait); 
  //  if(timeout && *timeout > ACE_Time_Value::zero)
  //  {
  //    //tq->dump(); 
  //    ACE_OS::sleep(timeout->sec()); 
  //  }

  //  ACE_ASSERT(timeout == 0 || timeout->sec() == 0 || timeout->sec() == 1); 
  //  tq->expire(); 
  //}

  //ACE_Thread_Manager::instance()->wait(); 
  //ACE_OS::sleep(10); 
  while(_running)
    ACE_OS::sleep(1); 

  tq->deactivate(); 
  ACE_Thread_Manager::instance()->join(tq->thr_id()); 

  void const* act = 0; 
  for(int i=0; i<size; ++ i)
  {
    if(tq->cancel(ids[i], &act) == 1)
      delete (timer_handler*)act; 
    else 
      ACE_DEBUG((LM_DEBUG, "losing timer %d.\n", ids[i])); 
  }

  return 0; 
}

void end_loop(int signum/*, siginfo_t *siginfo, ucontext_t *ucontext*/)
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}



int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("the main thread %t has started.\n"))); 

  _running = 1; 
  ActiveTimer act_timer; 
  act_timer.activate(); 

  ACE_Sig_Action sa(end_loop, SIGINT);   
  test_timer_queue(&act_timer);

	return 0;
}

