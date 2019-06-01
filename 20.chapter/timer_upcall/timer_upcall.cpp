// timer_upcall.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Timer_Queue_T.h" 
#include "ace/Timer_Heap_T.h" 
#include <algorithm> 
#include "ace/Signal.h" 
#include "ace/os_ns_unistd.h" 

//#define SKIP_CANCELLATION 

int _running = 1; 

class timer_handler
{
public:
  timer_handler() : id_(-1), count_(0) { }  
  void timer_id(long id) { id_ = id; } 
  long timer_id() const { return id_; } 

  virtual int handle_event(void const* arg)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle_event %d (%d).\n"), id_, count_)); 
    ++ count_;
    return 0; 
  }

  virtual int handle_cancel()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle_cancel %d (%d).\n"), id_, count_)); 
    return 0; 
  }

  virtual int handle_close()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle_close %d (%d).\n"), id_, count_)); 
    return 0; 
  }

private:
  long id_; 
  int count_; 
}; 

class upcall_handler; 
typedef ACE_Timer_Queue_T<timer_handler*, upcall_handler, ACE_Null_Mutex> timer_queue; 
typedef ACE_Timer_Heap_T<timer_handler*, upcall_handler, ACE_Null_Mutex> timer_heap; 

class upcall_handler
{
public:
  int registration(timer_queue& queue
    , timer_handler* handler
    , void const* arg)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::registration")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d registration.\n"), handler->timer_id())); 
    return 0; 
  }

  int preinvoke(timer_queue& queue
    , timer_handler* handler
    , void const* arg
    , int recurring
    , ACE_Time_Value const& current
    , void const*& update_act)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::preinvoke")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d preinvoke.\n"), handler->timer_id())); 
    return 0; 
  }

  int postinvoke(timer_queue& queue
    , timer_handler* handler
    , void const* arg
    , int recurring
    , ACE_Time_Value const& current
    , void const* update_act)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::postinvoke")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d postinvoke.\n"), handler->timer_id())); 
    return 0; 
  }

  int timeout(timer_queue& queue
    , timer_handler* handler
    , void const* arg
    , int recuring
    , ACE_Time_Value const& current)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::timeout")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d time out.\n"), handler->timer_id())); 
    return handler->handle_event(arg); 
  }

  //int cancellation(timer_queue& queue
  //  , timer_handler* handler)
  //{
  //  ACE_TRACE(ACE_TEXT("upcall_handler::cancellation")); 
  //  ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d has been cancelled.\n"), handler->timer_id())); 
  //  return handler->handle_cancel(); 
  //}

  int cancel_timer(timer_queue& queue
    , timer_handler* handler
    , int dont_call
    , int require_ref_count)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::cancel_timer")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d cancel timer.\n"), handler->timer_id())); 
    return handler->handle_cancel(); 
  }

  int cancel_type(timer_queue& queue
    , timer_handler* handler
    , int dont_call
    , int& require_ref_count)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::cancel_type")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d cancel type.\n"), handler->timer_id())); 
    return handler->handle_close(); 
  }

  int deletion(timer_queue& queue, timer_handler* handler, void const* arg)
  {
    ACE_TRACE(ACE_TEXT("upcall_handler::deletion")); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("handler %d has been deleted.\n"), handler->timer_id())); 
    return handler->handle_close(); 
  }
}; 

int test_timer_queue(timer_queue* tq)
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
    future_time[i] = tq->gettimeofday() + (ACE_Time_Value)val[i]; 
    interval[i] = val[size-i-1]; 
  }

  for(int i=0; i<size; ++ i)
  {
    ACE_NEW_RETURN(handler, timer_handler, -1); 
    ids[i] = tq->schedule(handler, handler, future_time[i], interval[i]); 
    if(ids[i] == -1)
      return -1; 

    handler->timer_id(ids[i]); 
    //handler->interval(interval[i].sec()); 
  }

  ACE_Time_Value max_wait(1); 
  while(_running)
  {
    ACE_Time_Value* timeout = tq->calculate_timeout(&max_wait); 
    if(timeout && *timeout > ACE_Time_Value::zero)
    {
      //tq->dump(); 
      ACE_OS::sleep(timeout->sec()); 
    }

    ACE_ASSERT(timeout == 0 || timeout->sec() == 0 || timeout->sec() == 1); 
    tq->expire(); 
  }

#if !defined(SKIP_CANCELLATION)
  void const* act = 0; 
  for(int i=0; i<size; ++ i)
  {
    if(tq->cancel(ids[i], &act) == 1)
      delete (timer_handler*)act; 
    else 
      ACE_DEBUG((LM_DEBUG, "losing timer %d.\n", ids[i])); 
  }
#endif

  return 0; 
}

void end_loop(int signum/*, siginfo_t *siginfo, ucontext_t *ucontext*/)
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  timer_queue* queue = 0; 
  ACE_NEW_RETURN(queue, timer_heap(), -1); 

  ACE_Sig_Action sa(end_loop, SIGINT); 
  test_timer_queue(queue); 

  delete queue; 
	return 0;
}

