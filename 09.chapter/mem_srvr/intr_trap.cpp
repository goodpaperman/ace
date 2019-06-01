#include "StdAfx.h"
#include "intr_trap.h"

intr_trap::intr_trap(int signum, ACE_Reactor* r /* =ACE_Reactor::instance() */)
: signum_(signum)
{
  reactor(r); 
  reactor()->register_handler(signum_, this); 
}

intr_trap::~intr_trap(void)
{
  reactor()->remove_handler(signum_, 0); 
  signum_ = 0; 
}

int intr_trap::handle_signal(int signum, siginfo_t* /*=0*/, ucontext_t* /*=0*/)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  reactor()->end_reactor_event_loop(); 
  return 0; 
}
