// ac_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ACE_Event_Acceptor.h" 
#include "ace/WFMO_Reactor.h" 
#include "ace/SPIPE_Addr.h"
#include "ace/SPIPE_Acceptor.h" 
#include "Client_Service.h" 
#include "intr_trap.h" 

// #define REACTOR_ON_HEAP
// #define ACCEPTOR_ON_HEAP

typedef ACE_Event_Acceptor<Client_Service, ACE_SPIPE_ACCEPTOR> Client_Acceptor; 

#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Event_Acceptor<Client_Service, ACE_SPIPE_ACCEPTOR>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Event_Acceptor<Client_Service, ACE_SPIPE_ACCEPTOR>; 
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

  Client_Acceptor *acceptor = 0; 
  ACE_SPIPE_Addr local_addr("spipe"); 
#if defined(ACCEPTOR_ON_HEAP)
  ACE_NEW_RETURN(acceptor, Client_Acceptor(), -1); 
#else 
  Client_Acceptor acceptor_i; 
  acceptor = &acceptor_i; 
#endif 

  //acceptor->reactor(reactor); 
  if(acceptor->open(local_addr, reactor) == -1)
    return -1; 

  //reactor->remove_handler(acceptor->get_handle(), ACE_Event_Handler::ACCEPT_MASK); 
  //if(reactor->register_handler(acceptor, acceptor->get_handle()) == -1)
  //  return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  intr_trap sigint(SIGINT, reactor); 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

#if defined(ACCEPTOR_ON_HEAP)
  delete acceptor; 
#endif 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

  return 0;
}

