// suspend.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_CommandHandler.h" 
#include "ace/OS_NS_unistd.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  HA_CommandHandler handler; 
  int result = handler.activate(THR_NEW_LWP|THR_JOINABLE|THR_SUSPENDED); 
  ACE_ASSERT(result == 0); 
  //ACE_OS::sleep(10); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) The current thread count is %d\n"), handler.thr_count())); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) The group identifier is %d\n"), handler.grp_id())); 
  handler.resume(); 
  handler.wait(); 
	return 0;
}

