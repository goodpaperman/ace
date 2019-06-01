// MsgQueNT.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Message_Queue.h" 
#include "ace/Thread_Manager.h" 
#include "ace/os_ns_unistd.h" 

ACE_THR_FUNC_RETURN enqueue_proc(void *arg)
{
  ACE_Message_Queue_NT *msgque = (ACE_Message_Queue_NT *)arg; 
  ACE_Time_Value tv(0, 100); 
  while(!msgque->deactivated())
  {
    ACE_Message_Block *mb = new ACE_Message_Block(32); 
    mb->copy("hello world!\n"); 
    if(msgque->enqueue(mb) == -1)
    {
      mb->release(); 
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("enqueue")), -1); 
    }
    else 
      ACE_OS::sleep(tv); 
  }

  return 0; 
}

ACE_THR_FUNC_RETURN dequeue_proc(void *arg)
{
  ACE_Message_Queue_NT *msgque = (ACE_Message_Queue_NT *)arg; 
  ACE_Time_Value tv(0, 1000); 
  while(!msgque->deactivated())
  {
    ACE_Message_Block *mb = 0; 
    if(msgque->dequeue(mb, &tv) != -1)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("dequeue (%d, %d): %s"), 
        msgque->message_count(), 
        msgque->message_length(), 
        mb->base())); 

      mb->release(); 
    }
  }

  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Message_Queue_NT msgque; 

  if(ACE_Thread_Manager::instance()->spawn(enqueue_proc, &msgque) == -1)
    return -1; 

  if(ACE_Thread_Manager::instance()->spawn(dequeue_proc, &msgque) == -1)
    return -1; 

  ACE_OS::sleep(1); 
  msgque.dump(); 
  msgque.deactivate(); 
  ACE_Thread_Manager::instance()->wait(); 

  // cleanup the completion I/O port.
  msgque.activate(); 
  ACE_Message_Block *mb = 0;
  ACE_Time_Value tv(0, 1000); 
  int total = 0 ;
  while(msgque.dequeue(mb, &tv) != -1)
  {
    if(mb)
      mb->release(); 

    ++ total; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("clean total %d messages.\n"), total)); 
	return 0;
}

