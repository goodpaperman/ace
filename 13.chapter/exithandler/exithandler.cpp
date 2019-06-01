// exithandler.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_CommandHandler.h" 
#include "ace/OS_NS_Thread.h" 

//#define THR_MGR_ON_STACK

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ExitHandler eh; 
  HA_CommandHandler handler(eh); 

#if defined(THR_MGR_ON_STACK)
  ACE_Thread_Manager mgr; 
  handler.thr_mgr(&mgr);
#endif 

  handler.activate(); 

  //handler.wait(); 
#if defined(THR_MGR_ON_STACK)
  mgr.wait(); 
#else 
  ACE_Thread_Manager::instance()->wait(); 
#endif 

  return 0;
}