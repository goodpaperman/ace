#include "StdAfx.h"
#include "SignalableTask.h"
#include "ace/OS_NS_time.h" 

SignalableTask::SignalableTask(void)
: running_(1)
{
}

int SignalableTask::handle_signal(int signum, siginfo_t*, ucontext_t*)
{
  if(signum == SIGINT)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) received a %S signal\n"), signum)); 
    running_ = 0; 
  }

  return 0; 
}

int SignalableTask::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) starting thread\n"))); 

  while(running_)
  {
    ACE_Message_Block* mb = 0; 
    ACE_Time_Value tv(ACE_OS::gettimeofday() + ACE_Time_Value(0, 1000));  
    int result = getq(mb, &tv); 
    if(result == -1 && errno == EWOULDBLOCK)
      continue; 
    else 
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) processing message\n"))); 
      mb->release(); 
      mb = 0; 
    }
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) exit %u.\n"), running_)); 
  return 0; 
}