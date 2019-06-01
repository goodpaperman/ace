// HAClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Proactor.h" 
#include "ace/INET_Addr.h" 
#include "ace/Asynch_Connector.h" 
#include "ace/signal.h" 
#include "HA_Handler.h" 

ACE_Proactor* proactor = 0; 
void end_loop(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl + C pressed by user, exiting...\n"))); 
  proactor->proactor_end_event_loop(); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //ACE_Proactor* proactor = 0; 
  ACE_NEW_RETURN(proactor, ACE_Proactor(), -1); 

  ACE_INET_Addr peer_addr(8813, "127.0.0.1"); 
  ACE_Asynch_Connector<HA_Handler> connector;
  if(connector.open(1, proactor, 1) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open")), -1); 

  if(connector.connect(peer_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connect")), -1); 

  ACE_Sig_Action sa(end_loop); 
  sa.register_action(SIGINT); 
  proactor->proactor_run_event_loop(ACE_Proactor::check_reconfiguration); 

  delete proactor; 
	return 0;
}

