// tp_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "react_acceptor.h" 
//#include "intr_trap.h" 
#include "react_task.h" 
#include "ace/TP_Reactor.h" 

// #define REACTOR_ON_HEAP

int _running = 1; 
void end_loop(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Reactor *reactor = 0; 
#if defined(REACTOR_ON_HEAP)
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_TP_Reactor(), 1), -1); 
#else
  ACE_Reactor reactor_i(new ACE_TP_Reactor(), 1); 
  reactor = &reactor_i; 
#endif 

  react_acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 
  ACE_NEW_RETURN(acceptor, react_acceptor(), -1); 

  acceptor->reactor(reactor); 
  if(acceptor->open(port_to_listen) == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  ACE_Sig_Action sa(end_loop); 
  sa.register_action(SIGINT); 

  react_task* task = 0; 
  ACE_NEW_RETURN(task, react_task(), -1); 
  task->reactor(reactor); 
  task->activate(0, 10); 
  
  while(_running)
    ACE_OS::sleep(1); 

  //reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 
  reactor->end_reactor_event_loop(); 
  //ACE_Thread_Manager::instance()->cancel_task(task); 
  task->wait(); 
  delete task; 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

  return 0;
}

