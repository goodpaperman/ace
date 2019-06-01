// mt_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "react_acceptor.h" 
#include "intr_trap.h" 
#include "acceptor_task.h" 
#include "ace/Select_Reactor.h" 

//#define REACTOR_ON_HEAP

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Reactor *reactor = 0; 
#if defined(REACTOR_ON_HEAP)
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_Select_Reactor(), 1), -1); 
#else
  ACE_Reactor reactor_i(new ACE_Select_Reactor(), 1); 
  reactor = &reactor_i; 
#endif 

  acceptor_task* task = 0; 
  ACE_NEW_RETURN(task, acceptor_task(), -1); 
  task->reactor(reactor); 
  task->activate(); 
  
  intr_trap sigint(SIGINT, reactor); 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 
  ACE_Thread_Manager::instance()->cancel_task(task); 
  task->wait(); 
  delete task; 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

  return 0;
}

