#include "StdAfx.h"
#include "event_task.h"
#include "ace/Reactor.h" 

event_task::event_task(void)
{
}

event_task::~event_task(void)
{
}

int event_task::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) serve your command.\n"))); 
  reactor()->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 
  return 0; 
}