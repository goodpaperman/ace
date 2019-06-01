// HomeAutomation.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/proactor.h" 
#include "ace/signal.h" 
#include "HA_Acceptor.h" 

ACE_Proactor* proactor = 0; 
void end_loop(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl + C pressed by user, exiting...\n"))); 
  proactor->proactor_end_event_loop(); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_NEW_RETURN(proactor, ACE_Proactor(), -1); 

  HA_Acceptor* acceptor = 0; 
  ACE_NEW_RETURN(acceptor, HA_Acceptor(), -1); 
  ACE_INET_Addr listen_addr(8813); 
  if(acceptor->open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, proactor, 1, 1, -1) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("acceptor open")), -1); 
  
  ACE_Sig_Action sa(end_loop); 
  sa.register_action(SIGINT); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  proactor->proactor_run_event_loop(ACE_Proactor::check_reconfiguration); 

  delete acceptor; 
  delete proactor; 
	return 0;
}

