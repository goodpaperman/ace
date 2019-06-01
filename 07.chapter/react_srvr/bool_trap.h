#pragma once
#include "ace/Reactor.h"

class bool_trap : public ACE_Event_Handler
{
public:
  bool_trap(int sigon, int sigoff, ACE_Reactor* r = ACE_Reactor::instance());
  virtual ~bool_trap(void);

  virtual int handle_signal(int signum, siginfo_t* = 0, ucontext_t* = 0); 
  virtual int handle_exception(ACE_HANDLE handle = ACE_INVALID_HANDLE); 

private:
  int sigon_; 
  int sigoff_; 
  int on_; 
};
