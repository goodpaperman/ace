// sigaction.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Signal.h" 
#include "ace/os_ns_unistd.h" 

#define MAP_USER_SIGNALS
#define RAISE_SIGNAL

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

static void sig_handler(int signum)
{
  ACE_TRACE(ACE_TEXT("sig_handler")); 

#if defined(NESTING_SIGNAL)
#  if defined(ACE_WIN32)
  ::raise(SIGUSR1); 
#  else 
  ACE_OS::kill(ACE_OS::getpid(), SIGUSR1);
#  endif 
#endif 

  if(signum == SIGUSR1)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Signal SIGUSR1\n"))); 
  else 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Signal SIGUSR2\n"))); 

  ACE_OS::sleep(1); 
}

static void register_actions()
{
  ACE_TRACE(ACE_TEXT("register_actions")); 
  
  ACE_Sig_Set ss; 
  ss.sig_add(SIGUSR1); 

  //sa.mask(ss); 
  ACE_Sig_Action sa(sig_handler, ss); 
  sa.register_action(SIGUSR1); 
  sa.register_action(SIGUSR2); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_TRACE(ACE_TEXT("main")); 
  register_actions(); 

#if defined(RAISE_SIGNAL)
#  if defined(ACE_WIN32)
  ::raise(SIGUSR2); 
  ::raise(SIGUSR1); 
#  else 
  ACE_OS::kill(ACE_OS::getpid(), SIGUSR2); 
  ACE_OS::kill(ACE_OS::getpid(), SIGUSR1); 
#  endif 
#endif 

  while(ACE_OS::sleep(10) == -1)
  {
    if(errno == EINTR)
      continue; 
    else 
      ACE_OS::exit(1); 
  }

	return 0;
}

