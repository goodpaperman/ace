// logy.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/Service_Config.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  if(ACE_Service_Config::open(argc, argv, ACE_DEFAULT_LOGGER_KEY, 1, 0, 1) < 0)
  //if(ACE_Service_Config::open(argc, argv) < 0)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("Service Config open")), 1); 

  ACE_TRACE(ACE_TEXT("main")); 

  int n = 0; 
  ACE_Time_Value tv(0, 10); 
  while(n++ < 100000)
  {
    ACE_DEBUG((LM_NOTICE, ACE_TEXT("%t%IHowdy Pardner\n"))); 
    ACE_DEBUG((LM_INFO, ACE_TEXT("%t%IGoodnight\n"))); 
    ACE_Reactor::run_event_loop (tv);
  }

  ACE_Reactor::end_event_loop(); 
	return 0;
}

