#pragma once

#include "ace/Task_T.h" 

#define ASYNCH_CANCEL

#if defined(ASYNCH_CANCEL)
#  if !defined(PTHREAD_CANCEL_ENABLE)
#    define PTHREAD_CANCEL_ENABLE 1
#  endif 

#  if !defined(PTHREAD_CANCEL_ASYNCHRONOUS)
#    define PTHREAD_CANCEL_ASYNCHRONOUS 2
#  endif 
#endif 

class Cancellable_Task : public ACE_Task<ACE_MT_SYNCH>
{
public:
  virtual int svc(); 
};
