// ac_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Acceptor.h" 
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h" 
#include "Client_Service.h" 
#include "intr_trap.h" 

#define ON_HEAP
//#define DISABLE_NOTIFY
#define USE_TP_REACTOR

typedef ACE_Acceptor<Client_Service, ACE_SOCK_ACCEPTOR> Client_Acceptor; 

#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Acceptor<Client_Service, ACE_SOCK_ACCEPTOR>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Acceptor<Client_Service, ACE_SOCK_ACCEPTOR>; 
#endif 

#if defined (USE_TP_REACTOR)
#include "ace/TP_Reactor.h" 
#else 
#include "ace/Select_Reactor.h" 
#endif 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int disable_notify = 0; 
#if defined (DISABLE_NOTIFY)
  disable_notify = 1; 
#endif 

  ACE_Reactor *reactor = 0; 
#if defined(ON_HEAP)
#  if defined (USE_TP_REACTOR) 
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_TP_Reactor(), 1), -1); 
#  else
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_Select_Reactor(0, 0, disable_notify), 1), -1); 
#  endif 
#else
#  if defined (USE_TP_REACTOR)
  ACE_Reactor reactor_i(new ACE_TP_Reactor(), 1); 
#  else
  ACE_Reactor reactor_i(new ACE_Select_Reactor(0, 0, disable_notify), 1); 
#  endif 
  reactor = &reactor_i; 
#endif 

  Client_Acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 
#if defined(ON_HEAP)
  ACE_NEW_RETURN(acceptor, Client_Acceptor(), -1); 
#else 
  Client_Acceptor acceptor_i; 
  acceptor = &acceptor_i; 
#endif 

  //acceptor->reactor(reactor); 
  if(acceptor->open(port_to_listen, reactor) == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) acceptor opened.\n"))); 

#if defined (ON_HEAP)
  Timer *timer = new Timer (ACE_Thread_Manager::instance ()); 
#else 
  Timer t (ACE_Thread_Manager::instance ()), *timer = &t;
#endif 
  timer->reactor (reactor); 
  timer->activate (); 

#if defined (ON_HEAP)
  intr_trap *sigint = 0; 
  ACE_NEW_RETURN (sigint, intr_trap (SIGINT, reactor), -1); 
#else 
  intr_trap sigint(SIGINT, reactor); 
#endif 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

  ACE_Thread_Manager::instance ()->cancel_task (timer); 
  timer->wait (); 

#if defined(ON_HEAP)
  delete timer; 
  delete sigint; 
  delete acceptor; 
  delete reactor; 
#endif 

  return 0;
}

