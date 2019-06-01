#include "StdAfx.h"
#include "bool_trap.h"
#include "ace/Signal.h" 

bool_trap::bool_trap(int sigon, int sigoff, ACE_Reactor* r /* =ACE_Reactor::instance() */)
: sigon_(sigon)
, sigoff_(sigoff)
, on_(0)
{
  reactor(r); 

  ACE_Sig_Set sigset; 
  sigset.sig_add(sigon_); 
  sigset.sig_add(sigoff_); 
  reactor()->register_handler(sigset, this); 
}

bool_trap::~bool_trap(void)
{
  ACE_Sig_Set sigset; 
  sigset.sig_add(sigon_); 
  sigset.sig_add(sigoff_); 
  reactor()->remove_handler(sigset); 
}

int bool_trap::handle_signal(int signum, siginfo_t* /*=0*/, ucontext_t* /*=0*/)
{
  on_ = (signum == sigon_); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("catch a \"%S\", log output will be %s.\n"), signum, on_ ? "ON" : "OFF")); 
  return reactor()->notify(this, EXCEPT_MASK); 
}

int bool_trap::handle_exception(ACE_HANDLE handle /* =INVALID_HANDLE_VALUE */)
{
  on_ 
    ? ACE_LOG_MSG->clr_flags(ACE_Log_Msg::SILENT) // clear SILENT to output
    : ACE_LOG_MSG->set_flags(ACE_Log_Msg::SILENT); // set SILENT to disable it.

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("log output now %s.\n"), on_ ? "ON" : "OFF")); 
  return 0; 
}
