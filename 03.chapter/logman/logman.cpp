// logman.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Log_Manager.h" 
#include "callback.h" 

void foo(void)
{
  ACE_TRACE(ACE_TEXT("foo")); 
  LOG_MANAGER->redirectToFile("logman.log"); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IHowdy Pardner\n"))); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  LOG_MANAGER->redirectToStderr(); 
  ACE_TRACE(ACE_TEXT("main")); 

  if(LOG_MANAGER->redirectToSyslog("logman.exe") != 0)
    printf("redirect to syslog failed.\n"); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IHi Mom\n"))); 

  foo(); 

  if(LOG_MANAGER->redirectToDaemon("logman.exe") != 0)
    printf("redirect to daemon failed.\n"); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IGoodnight\n"))); 


  Callback callback; 
  LOG_MANAGER->redirectToCallback(&callback); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IBye Bye\n"))); 

  LOG_MANAGER->redirectToStderr(); 
	return 0;
}

