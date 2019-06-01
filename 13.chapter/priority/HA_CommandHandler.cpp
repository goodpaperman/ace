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
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) starting up %C.\n"), name_)); 
  ACE_Message_Block *mb = 0; 
  int n = 0; 
  ACE_Time_Value tv(0, 1000), no_wait(ACE_OS::gettimeofday()); 
  while(this->getq(mb, &no_wait) != -1)
  {
    //ACE_OS::sleep(tv); 
    mb->release(); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) %C working %u\n"), name_, ++n)); 
  }

  //ACE_Time_Value tv(0, 100000); 
  //while(reactor()->reactor_event_loop_done() == 0)
  //{
  //  ACE_Message_Block* mb = 0; 
  //  if(getq(mb, &tv) != -1)
  //  {
  //    if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
  //    {
  //      ACE_DEBUG((LM_DEBUG, ACE_TEXT("begin shutting down...\n"))); 
  //      reactor()->end_reactor_event_loop(); 
  //    }
  //    else
  //    {
  //      DeviceCommandHeader* header = (DeviceCommandHeader*)mb->rd_ptr(); 
  //      ACE_DEBUG((LM_DEBUG, 
  //        ACE_TEXT("device id: %d\n")
  //        ACE_TEXT("length: %d\n")
  //        ACE_TEXT("    %s\n"), 
  //        header->device_id, 
  //        header->length, 
  //        (char *)(header + 1))); 
  //    }

  //    mb->release(); 
  //  }
  //}

  return 0; 
}