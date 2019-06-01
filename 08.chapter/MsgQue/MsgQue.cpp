// MsgQue.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Thread_Mutex.h" 
#include "ace/Condition_Thread_Mutex.h" 
#include "ace/Message_Queue_T.h" 
#include "ace/Thread_Manager.h" 
#include "ace/os_ns_unistd.h" 

#define TIN_HOUT_MODE 0 // tail in, head out
#define TOUT_HIN_MODE 1 // tail out, head in
#define IN_PRIO_MODE 2
#define IN_DEADL_MODE 3
#define OUT_PRIO_MODE 4
#define OUT_DEADL_MODE 5

#define QUEUE_MODE TIN_HOUT_MODE
//#define QUEUE_MODE TOUT_HIN_MODE
//#define QUEUE_MODE IN_PRIO_MODE
//#define QUEUE_MODE IN_DEADL_MODE
//#define QUEUE_MODE OUT_PRIO_MODE
//#define QUEUE_MODE OUT_DEADL_MODE

#define LACKS_AUTO_CLEANUP
#define CLEANUP_WITH_ITERATOR

ACE_THR_FUNC_RETURN enqueue_proc(void *arg)
{
  ACE_Message_Queue<ACE_MT_SYNCH> *que = (ACE_Message_Queue<ACE_MT_SYNCH> *)arg; 

  int order = 1; 
  ACE_Time_Value tv(0, 100); 
  while(!que->deactivated())
  {
    ACE_Message_Block *mb = new ACE_Message_Block(32); 
    //mb->copy("hello world!\n"); 
    sprintf(mb->base(), "%10d", order++); 
    mb->wr_ptr(strlen(mb->base())+1); 
    mb->msg_priority(-order); 
    mb->msg_deadline_time(ACE_OS::gettimeofday().sec() + order); 

#if ((QUEUE_MODE == TIN_HOUT_MODE) || \
     (QUEUE_MODE == OUT_PRIO_MODE) || \
     (QUEUE_MODE == OUT_DEADL_MODE))
    if(que->enqueue_tail(mb) == -1)
#elif (QUEUE_MODE == TOUT_HIN_MODE)
    if(que->enqueue_head(mb) == -1) 
#elif (QUEUE_MODE == IN_PRIO_MODE)
    if(que->enqueue_prio(mb) == -1)
#elif (QUEUE_MODE == IN_DEADL_MODE)
    if(que->enqueue_deadline(mb) == -1)
#endif 
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
  ACE_Message_Queue<ACE_MT_SYNCH> *que = (ACE_Message_Queue<ACE_MT_SYNCH> *)arg; 
  ACE_Time_Value tv(0, 1000); 
  while(!que->deactivated())
  {
    ACE_Message_Block *mb = 0; 
#if ((QUEUE_MODE == TIN_HOUT_MODE) || \
     (QUEUE_MODE == IN_PRIO_MODE) || \
     (QUEUE_MODE == IN_DEADL_MODE))
    if(que->dequeue_head(mb, &tv) != -1)
#elif (QUEUE_MODE == TOUT_HIN_MODE)
    if(que->dequeue_tail(mb, &tv) != -1)
#elif (QUEUE_MODE == OUT_PRIO_MODE)
    if(que->dequeue_prio(mb, &tv) != -1)
#elif (QUEUE_MODE == OUT_DEADL_MODE)
    if(que->dequeue_deadline(mb, &tv) != -1)
#endif 
    {
#if ((QUEUE_MODE == IN_PRIO_MODE) || \
     (QUEUE_MODE == OUT_PRIO_MODE))
      ACE_DEBUG((LM_DEBUG, 
        ACE_TEXT("dequeue (%10d, %10d): [%u] %s.\n"), 
        que->message_count(), 
        que->message_length(), 
        mb->msg_priority(), 
        mb->base())); 
#elif ((QUEUE_MODE == IN_DEADL_MODE) || \
       (QUEUE_MODE == OUT_DEADL_MODE))
      ACE_DEBUG((LM_DEBUG, 
        ACE_TEXT("dequeue (%10d, %10d): [%d] %s.\n"), 
        que->message_count(), 
        que->message_length(), 
        mb->msg_deadline_time().sec(), 
        mb->base())); 
#else 
      ACE_DEBUG((LM_DEBUG, 
        ACE_TEXT("dequeue (%10d, %10d): %s.\n"), 
        que->message_count(), 
        que->message_length(), 
        mb->base())); 
#endif 

      mb->release(); 
    }
  }

  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Message_Queue<ACE_MT_SYNCH> que; 
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
#    if ((QUEUE_MODE == IN_PRIO_MODE) || \
     (QUEUE_MODE == OUT_PRIO_MODE))
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%u] %s.\n"), mb->msg_priority(), mb->base())); 
#    elif ((QUEUE_MODE == IN_DEADL_MODE) || \
       (QUEUE_MODE == OUT_DEADL_MODE))
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] %s.\n"), mb->msg_deadline_time().sec(), mb->base())); 
#    else 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s.\n"), mb->base())); 
#    endif 

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
#    if ((QUEUE_MODE == IN_PRIO_MODE) || \
     (QUEUE_MODE == OUT_PRIO_MODE))
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%u] %s.\n"), mb->msg_priority(), mb->base())); 
#    elif ((QUEUE_MODE == IN_DEADL_MODE) || \
       (QUEUE_MODE == OUT_DEADL_MODE))
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] %s.\n"), mb->msg_deadline_time().sec(), mb->base())); 
#    else 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s.\n"), mb->base())); 
#    endif 
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

