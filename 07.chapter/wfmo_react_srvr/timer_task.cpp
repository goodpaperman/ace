#include "StdAfx.h"
#include "timer_task.h"
#include "ace/Reactor.h" 
#include "ace/os_ns_unistd.h" 

timer_task::timer_task(void)
{
}

timer_task::~timer_task(void)
{
}

int timer_task::svc()
{
  ACE_OS::sleep(1); 

  int timer_id = reactor()->schedule_timer(this, 0, 2, 3); 
  if(timer_id == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("register timer event %d.\n"), timer_id)); 
  reactor()->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 
  reactor()->cancel_timer(timer_id); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("cancel timer event %d.\n"), timer_id)); 
  return 0; 
}

int timer_task::handle_timeout(ACE_Time_Value const& tv, void const* act /* = 0 */)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) timeout.\n"))); 
  return 0; 
}