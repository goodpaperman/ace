// hook.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_ThreadHook.h" 
#include "HA_CommandHandler.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  HA_ThreadHook hook; 
  ACE_Thread_Hook::thread_hook(&hook); 

  HA_CommandHandler handler("HA"); 
  handler.putq(new ACE_Message_Block("hello world!\n")); 
  handler.activate(); 
  handler.wait(); 
	return 0;
}

