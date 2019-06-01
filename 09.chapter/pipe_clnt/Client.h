#pragma once

#include "ace/Svc_Handler.h" 
#include "ace/INET_Addr.h" 
#include "ace/MEM_Stream.h" 
#include "ace/Reactor_Notification_Strategy.h" 

class Client : public ACE_Svc_Handler<ACE_MEM_STREAM, ACE_NULL_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_MEM_STREAM, ACE_NULL_SYNCH> PARENT; 
  enum { ITERATIONS = 5 }; 

public:
  Client(void); 
  virtual ~Client(void);
  virtual int open(void * =0); 
  virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_timeout(ACE_Time_Value const& tv, void const* act = 0); 

private:
  int iterations_; 
  ACE_Reactor_Notification_Strategy notifier_; 
};
