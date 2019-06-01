// cancel.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Cancellable_Task.h" 
#include "ace/OS_NS_unistd.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  Cancellable_Task task; 
  task.putq(new ACE_Message_Block("Hello World!")); 
  task.activate(); 

  ACE_OS::sleep(1); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) cancelling task.\n"))); 
#if defined(ASYNCH_CANCEL)
  ACE_Thread_Manager::instance()->cancel_task(&task, 1); 
#else 
  ACE_Thread_Manager::instance()->cancel_task(&task); 
#endif 
  task.wait(); 
	return 0;
}

