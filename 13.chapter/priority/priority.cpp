// priority.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_CommandHandler.h" 
#include "ace/OS_NS_Thread.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  HA_CommandHandler hp_handler("HighPriority"); 
  hp_handler.activate(THR_NEW_LWP|THR_JOINABLE|THR_SUSPENDED, 1, 1, ACE_THR_PRI_OTHER_MAX); 

  HA_CommandHandler lp_handler("LowPriority"); 
  lp_handler.activate(THR_NEW_LWP|THR_JOINABLE|THR_SUSPENDED, 1, 1, ACE_THR_PRI_OTHER_MIN); 

  ACE_Message_Block* mb = 0; 
  for(int i=0; i<100; ++i)
  {
    ACE_NEW_RETURN(mb, ACE_Message_Block(), -1); 
    hp_handler.putq(mb); 
    
    ACE_NEW_RETURN(mb, ACE_Message_Block(), -1); 
    lp_handler.putq(mb); 
  }

  hp_handler.resume(); 
  lp_handler.resume(); 

  hp_handler.wait(); 
  lp_handler.wait(); 
	return 0;
}

