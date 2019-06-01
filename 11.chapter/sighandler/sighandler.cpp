// sighandler.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Signal.h" 
#include "ace/Event_Handler.h" 
#include "ace/os_ns_unistd.h" 

#define MAP_USER_SIGNALS
#define RAISE_SIGNAL
//#define ACE_HAS_SIGINFO_T

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
#if !defined(ACE_HAS_SIGINFO_T)
    ACE_ASSERT(signum == signum_); 
#endif 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%S occured\n"), signum)); 
    if(siginfo == 0)
    {
      ACE_DEBUG((LM_INFO, ACE_TEXT("No siginfo_t available for [%S]\n"), signum)); 
      return 0; 
    }

#if defined(ACE_HAS_SIGINFO_T)
    ACE_DEBUG((LM_INFO, ACE_TEXT("errno for this signal = %d [%s]\n"), siginfo->si_errno, strerror(siginfo->si_errno))); 
    ACE_DEBUG((LM_INFO, ACE_TEXT("signal was sent by process %d / user %d\n"), siginfo->si_pid, siginfo->si_uid)); 
    switch(siginfo->si_code)
    {
    case SI_TIMER:
      ACE_DEBUG((LM_INFO, ACE_TEXT("timer expiration\n"))); 
      break; 
    case SI_USER:
      ACE_DEBUG((LM_INFO, ACE_TEXT("sent by kill, sigsend or raise\n"))); 
      break; 
    case SI_KERNEL:
      ACE_DEBUG((LM_INFO, ACE_TEXT("sent by kernel\n"))); 
      break; 
    }; 

    switch(signum)
    {
    case SIGFPE:
      switch(siginfo->si_code)
      {
      case FPE_INTDIV:
      case FPE_FLTDIV:
        ACE_DEBUG((LM_INFO, ACE_TEXT("divide by zero at %@\n"), siginfo->si_addr)); 
        break; 
      case FPE_INTOVF:
      case FPE_FLTOVF:
        ACE_DEBUG((LM_INFO, ACE_TEXT("numeric overflow at %@\n"), siginfo->si_addr)); 
        break; 
      }; 
      break; 
    case SIGSEGV:
      switch(siginfo->si_code)
      {
      default:
        ACE_DEBUG((LM_INFO, ACE_TEXT("segment fault at %@\n"), siginfo->si_addr)); 
        break; 
      }; 
      break; 
    case SIGCHILD:
      ACE_DEBUG((LM_INFO, ACE_TEXT("a child process has exited\nthe child consumed %l/%l time\nand exited with value %d\n")
        , siginfo->si_utime, siginfo->si_stime, siginfo->si_status)); 
      break; 
    default:
      ACE_DEBUG((LM_INFO, ACE_TEXT("no description provided for this signal\n"))); 
      break; 
    }; 
#endif // ACE_HAS_SIGINFO_T

    return 0; 
  }

private:
  int signum_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_TRACE(ACE_TEXT("main")); 
  //register_actions(); 
  ACE_Sig_Handler handler; 
  sig_handler* h = 0; 

#if defined(ACE_HAS_SIGINFO_T)
  ACE_NEW_RETURN(h, sig_handler(SIGINT), -1); 
  ACE_Sig_Set ss; 
  //ss.fill_set(); 
  ss.sig_add(SIGINT); 
  ss.sig_add(SIGILL); 
  ss.sig_add(SIGFPE); 
  ss.sig_add(SIGSEGV); 
  ss.sig_add(SIGTERM); 
  ss.sig_add(SIGBREAK); 
  ss.sig_add(SIGABRT); 
  for(int i=0; i<ACE_NSIG; ++ i)
    if(ACE_OS::sigismember(ss, i))
      handler.register_handler(i, h); 

#  if 0
  pid_t child = ACE_OS::fork(); 
  if(child < 0)
    return -1; 
  else if(child > 0)
    ACE_OS::sleep(10); 
#  endif 
#else 
  ACE_NEW_RETURN(h, sig_handler(SIGUSR1), -1); 
  handler.register_handler(SIGUSR1, h); 

  ACE_NEW_RETURN(h, sig_handler(SIGUSR2), -1); 
  handler.register_handler(SIGUSR2, h); 
#endif 

#if defined(RAISE_SIGNAL)
#  if defined(ACE_WIN32)
  ::raise(SIGUSR2); 
  ::raise(SIGUSR1); 
#    if defined(ACE_HAS_SIGINFO_T)
  ::raise(SIGFPE); 
  ::raise(SIGTERM); 
  ::raise(SIGILL); 
  ::raise(SIGSEGV); 
  ::raise(SIGABRT); 
  ::raise(SIGBREAK); 
  ::raise(SIGINT); 
  //int n = 0; 
  //float m = 4.0 / n; 
  //int p = 4 / n; 
#     endif 
#  else 
  ACE_OS::kill(ACE_OS::getpid(), SIGUSR2); 
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
#if defined(ACE_HAS_SIGINFO_T)
  if(handler.register_handler(SIGINT, 0, 0, &eh, 0) == 0)
    delete eh; 
#else 
  if(handler.register_handler(SIGUSR1, 0, 0, &eh, 0) == 0)
    delete eh; 
  if(handler.register_handler(SIGUSR2, 0, 0, &eh, 0) == 0)
    delete eh; 
#endif 

	return 0;
}

