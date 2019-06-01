// ac_react_srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Select_Reactor.h"
#include "HA_Acceptor.h" 
#include "intr_trap.h" 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Reactor *reactor = 0; 
  ACE_Reactor reactor_i(new ACE_Select_Reactor(), 1); 
  reactor = &reactor_i; 
  
  HA_CommandHandler ch; 
  ch.reactor(reactor); 

  HA_Acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 
  HA_Acceptor acceptor_i(&ch); 
  acceptor = &acceptor_i; 

  if(acceptor->open(port_to_listen, reactor) == -1)
    return -1; 

  if(ch.activate() == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  intr_trap sigint(SIGINT, reactor); 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

  ch.wait(); 
  return 0;
}

