#include "StdAfx.h"
#include "react_task.h"
#include "ace/Reactor.h" 

react_task::react_task(void)
{
}

react_task::~react_task(void)
{
}


int react_task::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("thread (%t) will serve for you.\n"))); 
  reactor()->run_reactor_event_loop(ACE_Reactor::check_reconfiguration); 
  return 0; 
}