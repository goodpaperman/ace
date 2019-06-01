// task.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/os_ns_unistd.h" 

class HA_CommandHandler : public ACE_Task_Base
{
public:
  virtual int svc()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Handler Thread running\n"))); 
    ACE_OS::sleep(4); 
    return 0; 
  }
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Main Thread running\n"))); 
  HA_CommandHandler handler; 
  int result = handler.activate(); 
  ACE_ASSERT(result == 0); 
  handler.wait(); 
	return 0;
}

