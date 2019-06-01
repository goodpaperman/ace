// signal.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SignalableTask.h" 
#include "ace/signal.h" 
#include "ace/OS_NS_unistd.h" 

#define PROCESS_SIGNAL


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  SignalableTask handler; 
  handler.activate(THR_NEW_LWP|THR_JOINABLE, 5); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Main thread\n"))); 

  ACE_Sig_Handler sh; 
  sh.register_handler(SIGINT, &handler); 

  ACE_OS::sleep(1); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("sending signal SIGINT\n"))); 
#if defined(PROCESS_SIGNAL)
  for(int i=0; i<5; ++ i)
    ACE_OS::kill(ACE_OS::getpid(), SIGINT); 
#else 
  ACE_Thread_Manager::instance()->kill_grp(handler.grp_id(), SIGINT); 
#endif 

  handler.wait(); 
	return 0;
}

