// sighandlers.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Signal.h" 
#include "ace/Event_Handler.h" 
#include "ace/os_ns_unistd.h" 

#define MAP_USER_SIGNALS
//#define RAISE_SIGNAL

#if defined(MAP_USER_SIGNALS)
#  if defined(SIGUSR1)
#    undef SIGUSR1
#  endif 

#  if defined(SIGUSR2)
#    undef SIGUSR2
#  endif 

#  define SIGUSR1 SIGINT
#  define SIGUSR2 SIGBREAK
#endif 

class sig_handler : public ACE_Event_Handler
{
public:
  sig_handler(int signum) : signum_(signum) { }
  virtual ~sig_handler() { } 
  virtual int handle_signal(int signum, siginfo_t* siginfo=0, ucontext_t* =0 )
  {
    ACE_TRACE(ACE_TEXT("sig_handler::handle_signal")); 
    ACE_ASSERT(signum == signum_); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%S occured in handler %@\n"), signum, this)); 
    return 0; 
  }

private:
  int signum_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_TRACE(ACE_TEXT("main")); 
  //register_actions(); 
  ACE_Sig_Handlers handler; 
  sig_handler* h = 0; 

  ACE_NEW_RETURN(h, sig_handler(SIGUSR1), -1); 
  int key1 = handler.register_handler(SIGUSR1, h); 
  if(key1 == -1)
    return -1; 

  ACE_NEW_RETURN(h, sig_handler(SIGUSR1), -1); 
  int key2 = handler.register_handler(SIGUSR1, h); 
  if(key2 == -1)
    return -1; 

#if defined(RAISE_SIGNAL)
#  if defined(ACE_WIN32)
  //::raise(SIGUSR2); 
  ::raise(SIGUSR1); 
#  else 
  //ACE_OS::kill(ACE_OS::getpid(), SIGUSR2); 
  ACE_OS::kill(ACE_OS::getpid(), SIGUSR1); 
#  endif 
#endif 

  while(ACE_OS::sleep(3) == -1)
  {
    if(errno == EINTR)
      continue; 
    else 
      ACE_OS::exit(1); 
  }

  ACE_Event_Handler* eh = 0; 
  if(handler.remove_handler(SIGUSR1, 0, 0, key1) == 0)
    ; //delete eh; 
  if(handler.remove_handler(SIGUSR1, 0, 0, key2) == 0)
    ; //delete eh; 

	return 0;
}

