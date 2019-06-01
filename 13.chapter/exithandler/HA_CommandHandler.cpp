#include "StdAfx.h"
#include "HA_CommandHandler.h"
#include "ace/OS_NS_unistd.h" 
//#include "DeviceCommandHeader.h" 
//#include "ace/Reactor.h" 

void ExitHandler::apply()
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) is exiting.\n")));   
}

HA_CommandHandler::HA_CommandHandler(ExitHandler& eh)
: eh_(eh)
{
}

HA_CommandHandler::~HA_CommandHandler(void)
{
}

int HA_CommandHandler::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) starting up.\n"))); 
  thr_mgr()->at_exit(eh_); 

  ACE_OS::sleep(2); 
  ACE_Thread::exit(); 
  return 0; 
}