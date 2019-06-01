#include "StdAfx.h"
#include "Cancellable_Task.h"

int Cancellable_Task::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) starting up\n"))); 

#if defined(ASYNCH_CANCEL)
  cancel_state new_state; 
  new_state.cancelstate = PTHREAD_CANCEL_ENABLE; 
  new_state.canceltype = PTHREAD_CANCEL_ASYNCHRONOUS; 
  if(ACE_Thread::setcancelstate(new_state, 0) == -1)
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("cancel state"))); 
#endif 

  ACE_Thread_Manager* mgr = this->thr_mgr(); 
  while(mgr->testcancel(mgr->thr_self()) == 0)
  {
    ACE_Message_Block* mb = 0; 
    ACE_Time_Value tv = ACE_OS::gettimeofday() + ACE_Time_Value(0, 1000); 
    int result = this->getq(mb, &tv); 
    if(result == -1 && errno == EWOULDBLOCK)
      continue; 
    else 
    {
      mb->release(); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) process message\n"))); 
    }
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) canceled, exiting.\n"))); 
  return 0; 
}