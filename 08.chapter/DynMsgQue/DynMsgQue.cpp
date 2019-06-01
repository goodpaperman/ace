// DynMsgQue.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Thread_Mutex.h" 
#include "ace/Message_Queue_T.h" 
#include "ace/Condition_Thread_Mutex.h" 
#include "ace/Thread_Manager.h" 
#include "ace/os_ns_unistd.h" 

#define LACKS_AUTO_CLEANUP
#define CLEANUP_WITH_ITERATOR
#define USE_LAXITY_STRATEGY

ACE_THR_FUNC_RETURN enqueue_proc(void *arg)
{
  ACE_Dynamic_Message_Queue<ACE_MT_SYNCH> *que = (ACE_Dynamic_Message_Queue<ACE_MT_SYNCH> *)arg; 

  int order = 1; 
  ACE_Time_Value tv(0, 100); 
  while(!que->deactivated())
  {
    ACE_Message_Block *mb = new ACE_Message_Block(32); 
    //mb->copy("hello world!\n"); 
    sprintf(mb->base(), "%10d", order++); 
    mb->wr_ptr(strlen(mb->base())+1); 
    mb->msg_priority(order); 
    mb->msg_execution_time(ACE_Time_Value(0, 100000)); 
    mb->msg_deadline_time(ACE_OS::gettimeofday() + ACE_Time_Value(0, 100000)); 

    if(que->enqueue(mb) == -1)
    {
      mb->release(); 
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("enqueue")), -1); 
    }
    //else 
    //  ACE_OS::sleep(tv); 
  }

  return 0; 
}

ACE_THR_FUNC_RETURN dequeue_proc(void *arg)
{
  ACE_Dynamic_Message_Queue<ACE_MT_SYNCH> *que = (ACE_Dynamic_Message_Queue<ACE_MT_SYNCH> *)arg; 
  ACE_Time_Value tv(0, 1000); 
  while(!que->deactivated())
  {
    ACE_Message_Block *mb = 0; 
    if(que->dequeue(mb, &tv) != -1)
    {
      ACE_DEBUG((LM_DEBUG, 
        ACE_TEXT("dequeue (%10d, %10d): [%u] %s.\n"), 
        que->message_count(), 
        que->message_length(), 
        mb->msg_priority(), 
        mb->base())); 

      mb->release(); 
    }
  }

  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Dynamic_Message_Strategy *strategy = 0; 
#if defined(USE_LAXITY_STRATEGY)
  strategy = new ACE_Laxity_Message_Strategy(); 
#else
  strategy = new ACE_Deadline_Message_Strategy() ; 
#endif 

  ACE_Dynamic_Message_Queue<ACE_MT_SYNCH> que(*strategy); 
  que.low_water_mark(8*1024); 
  que.high_water_mark(16 * 1024); 

  if(ACE_Thread_Manager::instance()->spawn(enqueue_proc, &que) == -1)
    return -1; 

  if(ACE_Thread_Manager::instance()->spawn(dequeue_proc, &que) == -1)
    return -1; 

  ACE_OS::sleep(10); 

  {
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, que.lock(), -1); 
    que.dump(); 
  }

  que.deactivate(); 
  ACE_Thread_Manager::instance()->wait(); 

  
  int total = 0 ;
#if defined(LACKS_AUTO_CLEANUP)
  ACE_Message_Block *mb = 0;
#  if defined(CLEANUP_WITH_ITERATOR)
  ACE_Message_Queue_Iterator<ACE_MT_SYNCH> iter(que); 
  for(; !iter.done(); iter.advance())
  {
    if(iter.next(mb) && mb)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%u] %s.\n"), mb->msg_priority(), mb->base())); 

      // we can not release message block here, 
      // as we need the next/prev pointer for iterating, 
      // so this is just a fake cleanup, 
      // all the cleanup work will be done by message queue flush
      // in close in dtor.
      //mb->release(); 
    }

    ++ total; 
  }
#  else
  // cleanup the completion I/O port.
  que.activate(); 
  ACE_Time_Value tv(0, 1000); 
  while(que.dequeue(mb, &tv) != -1)
  {
    if(mb)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%u] %s.\n"), mb->msg_priority(), mb->base())); 
      mb->release(); 
    }

    ++ total; 
  }
#  endif 
#else 
  total = que.flush(); 
#endif 
 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("clean total %d messages.\n"), total)); 
	return 0;
}

