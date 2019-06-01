#pragma once

#include "ace/Svc_Handler.h" 
#include "ace/SPIPE_Stream.h" 

class Client_Service : public ACE_Svc_Handler<ACE_SPIPE_STREAM, ACE_NULL_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_SPIPE_STREAM, ACE_NULL_SYNCH> PARENT; 

public:
  Client_Service(void);
  virtual ~Client_Service(void);
  
  virtual int open(void* = 0); 
  virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask); 
};
