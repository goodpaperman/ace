// HomeAutomation.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/proactor.h" 
#include "ace/signal.h" 
//#include "ace/SOCK_Dgram.h" 
#include "ace/SOCK_CODgram.h" 
#include "ace/Message_Block.h" 
#include "HA_Service.h" 

ACE_Proactor* proactor = 0; 
void end_loop(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl + C pressed by user, exiting...\n"))); 
  proactor->proactor_end_event_loop(); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_NEW_RETURN(proactor, ACE_Proactor(), -1); 

  //HA_Acceptor* acceptor = 0; 
  //ACE_NEW_RETURN(acceptor, HA_Acceptor(), -1); 
  //ACE_INET_Addr listen_addr(8813); 
  //if(acceptor->open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, proactor, 1, 1, -1) == -1)
  //  ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("acceptor open")), -1); 

  //ACE_SOCK_Dgram dgram; 
  //ACE_INET_Addr listen_addr(8813); 
  //if(dgram.open(listen_addr) == -1)
  //  ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("dgram open")), -1); 
  
  ACE_SOCK_CODgram dgram; 
  ACE_INET_Addr listen_addr(8813); 
  if(dgram.open(ACE_Addr::sap_any, listen_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connection oriented dgram open")), -1); 

  HA_Service* service = 0; 
  ACE_NEW_RETURN(service, HA_Service(), -1); 
  ACE_Message_Block mb; 
  service->proactor(proactor); 
  service->open(dgram.get_handle(), mb); 

  //{
  //  ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("service open"))); 
  //  dgram.close(); 
  //  return -1; 
  //}

  ACE_Sig_Action sa(end_loop); 
  sa.register_action(SIGINT); 
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  proactor->proactor_run_event_loop(ACE_Proactor::check_reconfiguration); 

  //delete acceptor; 
  delete proactor; 
	return 0;
}

