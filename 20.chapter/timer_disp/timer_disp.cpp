// timer_disp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "timer_dispather.h" 
#include "timer_handler.h" 
#include "ace/Signal.h" 

int _running = 1; 

void end_loop(int signum/*, siginfo_t *siginfo, ucontext_t *ucontext*/)
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //TIMER::instance(); 

  timer_handler th[10]; 
  long args[10] = { 0 }; 
  ACE_Time_Value now = ACE_OS::gettimeofday(); 
  for(int i=0; i<10; ++ i)
  {
    args[i] = TIMER::instance()->schedule(th+i, args+i, now + (ACE_Time_Value)1, 3); 
    th[i].timer_id(args[i]); 
  }

  ACE_Sig_Action sa(end_loop, SIGINT); 
  TIMER::instance()->wait_for_event(&_running); 

  for(int i=0; i<10; ++ i)
  {
    // to avoid crash when timers still in, 
    // but process exit when user press Ctrl+C.
    TIMER::instance()->cancel(th[i].timer_id()); 
  }

	return 0;
}

