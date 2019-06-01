#pragma once

#include "ace/Task_T.h" 

class SignalableTask : public ACE_Task<ACE_MT_SYNCH>
{
public:
  SignalableTask(); 
  virtual int handle_signal(int signum, siginfo_t* =0, ucontext_t* =0); 
  virtual int svc(); 

private:
  int running_; 
};
