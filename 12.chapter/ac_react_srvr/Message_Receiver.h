#pragma once

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h" 
#include "DeviceCommandHeader.h" 
#include "HA_CommandHandler.h" 

class Message_Receiver : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH> PARENT; 

public:
  Message_Receiver(HA_CommandHandler* handler = 0);
  virtual ~Message_Receiver(void);
  
  virtual int open(void* = 0); 
  virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask); 

  int read_header(DeviceCommandHeader* head); 
  int copy_payload(ACE_Message_Block* mb, int length); 
  ACE_Message_Block* shut_down_message(); 

private:
  HA_CommandHandler* handler_; 
};
