// ac_clnt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Client.h"
#include "ace/Reactor.h" 
#include "ace/Select_Reactor.h" 
#include "../ac_react_srvr/intr_trap.h" 
#include "ace/Connector.h" 
#include "ace/SOCK_Connector.h" 

//#define REACTOR_ON_HEAP 
//#define SVC_HANDLER_ON_HEAP

typedef ACE_Connector<Client, ACE_SOCK_CONNECTOR> Connector; 

#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Connector<Client, ACE_SOCK_CONNECTOR>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Connector<Client, ACE_SOCK_CONNECTOR>; 
#endif 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Reactor *reactor = 0; 
#if defined(REACTOR_ON_HEAP)
  ACE_NEW_RETURN(reactor, ACE_Reactor(new ACE_Select_Reactor(), 1), -1); 
#else
  ACE_Reactor reactor_i(new ACE_Select_Reactor(), 1); 
  reactor = &reactor_i; 
#endif 

  Client client, *p = 
#if defined(SVC_HANDLER_ON_HEAP)
    0; 
#else 
    &client; 
#endif 

  Connector connector(reactor); 
  ACE_INET_Addr port_to_connect(50000, ACE_LOCALHOST); 
  if(connector.connect(p, port_to_connect, ACE_Synch_Options(0, ACE_Time_Value(10))) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connect ")), 1); 

  intr_trap sigint(SIGINT, reactor);
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

#if defined(REACTOR_ON_HEAP)
  delete reactor; 
#endif 

	return 0;
}

