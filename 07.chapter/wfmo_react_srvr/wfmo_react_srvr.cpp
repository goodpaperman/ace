// wfmo_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "react_acceptor.h" 
#include "intr_trap.h" 
#include "ace/WFMO_Reactor.h" 

// #define REACTOR_ON_HEAP
//#define SPECIAL_TIME_THREAD
#define MULT_EVENT_THREAD

#if defined(SPECIAL_TIME_THREAD)
#include "timer_task.h" 
#elif defined(MULT_EVENT_THREAD)
#include "event_task.h" 
#endif 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Reactor *reactor = 0; 
#if defined(REACTOR_ON_HEAP)
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_WFMO_Reactor(), 1), -1); 
#else
  ACE_Reactor reactor_i(new ACE_WFMO_Reactor(), 1); 
  reactor = &reactor_i; 
#endif 

  react_acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 
  ACE_NEW_RETURN(acceptor, react_acceptor(), -1); 

  acceptor->reactor(reactor); 
  if(acceptor->open(port_to_listen) == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  intr_trap sigint(SIGINT, reactor); 

#if defined(SPECIAL_TIME_THREAD)
  timer_task* task = 0; 
  ACE_NEW_RETURN(task, timer_task(), -1); 
  task->reactor(reactor); 
  task->activate(); 
#elif defined(MULT_EVENT_THREAD)
  event_task* task = 0; 
  ACE_NEW_RETURN(task, event_task(), -1); 
  task->reactor(reactor); 
  task->activate(0, 10); 
#endif 

  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

#if defined(SPECIAL_TIME_THREAD) || defined(MULT_EVENT_THREAD)
  task->wait(); 
  delete task; 
#endif 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

  return 0;
}

