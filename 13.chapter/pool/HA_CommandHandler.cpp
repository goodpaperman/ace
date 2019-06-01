#include "StdAfx.h"
#include "HA_CommandHandler.h"
#include "ace/OS_NS_unistd.h" 
#include "ace/OS_NS_time.h" 

//#include "DeviceCommandHeader.h" 
//#include "ace/Reactor.h" 

HA_CommandHandler::HA_CommandHandler(char const* name)
: name_(name)
{
}

HA_CommandHandler::~HA_CommandHandler(void)
{
}

int HA_CommandHandler::svc()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) starting up.\n"))); 
  ACE_Message_Block *mb = 0; 
  int n = 0; 
  ACE_Time_Value tv(0, 1000), no_wait(ACE_OS::gettimeofday()); 
  while(this->getq(mb, &no_wait) != -1)
  {
    //ACE_OS::sleep(tv); 
    mb->release(); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) %C working %u\n"), name_, ++n)); 
  }

  return 0; 
}