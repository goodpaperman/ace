#pragma once
#include "ace/Reactor.h"

class intr_trap : public ACE_Event_Handler
{
public:
  intr_trap(int signum = SIGINT, ACE_Reactor* reactor = ACE_Reactor::instance());
  virtual ~intr_trap(void);

  virtual int handle_signal(int signum, siginfo_t* =0, ucontext_t* =0); 

private:
  int signum_; 
};
