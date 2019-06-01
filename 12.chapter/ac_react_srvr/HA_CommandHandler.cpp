#include "StdAfx.h"
#include "HA_CommandHandler.h"
#include "DeviceCommandHeader.h" 
#include "ace/Reactor.h" 
#include "ace/OS_NS_time.h" 

HA_CommandHandler::HA_CommandHandler(void)
{
}

HA_CommandHandler::~HA_CommandHandler(void)
{
}

int HA_CommandHandler::svc()
{
  ACE_Time_Value tv(0, 100000); 
  while(reactor()->reactor_event_loop_done() == 0)
  {
    ACE_Message_Block* mb = 0; 
    //tv = ACE_OS::time(0); 
    //tv += ACE_Time_Value(0, 100000); 
    tv = ACE_OS::gettimeofday() + ACE_Time_Value(0, 100000); 
    if(getq(mb, &tv) != -1)
    {
      if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
      {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("begin shutting down...\n"))); 
        reactor()->end_reactor_event_loop(); 
      }
      else
      {
        DeviceCommandHeader* header = (DeviceCommandHeader*)mb->rd_ptr(); 
        ACE_DEBUG((LM_DEBUG, 
          ACE_TEXT("device id: %d\n")
          ACE_TEXT("length: %d\n")
          ACE_TEXT("    %s\n"), 
          header->device_id, 
          header->length, 
          (char *)(header + 1))); 
      }

      mb->release(); 
    }
  }

  return 0; 
}