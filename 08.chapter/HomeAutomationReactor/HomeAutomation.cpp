// HomeAutomation.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/reactor.h" 
#include "ace/proactor.h" 
#include "ace/win32_proactor.h" 
#include "ace/signal.h" 
#include "HA_Acceptor.h" 

ACE_Reactor* reactor = 0; 
void end_loop(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl + C pressed by user, exiting...\n"))); 
  reactor->end_reactor_event_loop(); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_NEW_RETURN(reactor, ACE_Reactor(), -1); 
  
  ACE_WIN32_Proactor* win32_proactor = 0; 
  ACE_NEW_RETURN(win32_proactor, ACE_WIN32_Proactor(1, 1), -1); 
  
  ACE_Proactor* proactor = 0; 
  ACE_NEW_RETURN(proactor, ACE_Proactor(win32_proactor, 1), -1); 

  if(reactor->register_handler(win32_proactor, win32_proactor->get_handle()) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p"), ACE_TEXT("register proactor to reactor")), -1); 

  HA_Acceptor* acceptor = 0; 
  ACE_NEW_RETURN(acceptor, HA_Acceptor(), -1); 
  ACE_INET_Addr listen_addr(8813); 
  if(acceptor->open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, proactor, 1, 1, -1) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("acceptor open")), -1); 
  
  ACE_Sig_Action sa(end_loop); 
  sa.register_action(SIGINT); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  //proactor->proactor_run_event_loop(ACE_Proactor::check_reconfiguration); 
  reactor->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 

  delete acceptor; 
  reactor->remove_handler(win32_proactor, ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL); 

  delete proactor; 
  delete reactor; 
	return 0;
}

