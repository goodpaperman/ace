// pool.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_CommandHandler.h" 
#include "ace/OS_NS_Thread.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  HA_CommandHandler handler(""); 
  handler.activate(THR_NEW_LWP|THR_JOINABLE|THR_SUSPENDED, 4); 

  ACE_Message_Block* mb = 0; 
  for(int i=0; i<100; ++i)
  {
    ACE_NEW_RETURN(mb, ACE_Message_Block(), -1); 
    handler.putq(mb); 
  }

  handler.resume(); 
  handler.wait(); 
	return 0;
}

