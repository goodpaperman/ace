// react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "react_acceptor.h" 
#include "bool_trap.h" 
#include "intr_trap.h" 
#include "ace/Select_Reactor.h" 

//#define REACTOR_ON_HEAP

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  react_acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 

  ACE_Reactor *reactor = 0; 
#if defined(REACTOR_ON_HEAP)
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_Select_Reactor(), 1), -1); 
#else
  ACE_Reactor reactor_i(new ACE_Select_Reactor(), 1); 
  reactor = &reactor_i; 
#endif 

  ACE_NEW_RETURN(acceptor, react_acceptor(), -1); 
  acceptor->reactor(reactor); 
  if(acceptor->open(port_to_listen) == -1)
    return -1; 
  
  intr_trap sigint(SIGINT, reactor); 
  bool_trap sigbool(SIGABRT, SIGTERM, reactor); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

  return 0;
}

