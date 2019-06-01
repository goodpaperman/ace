#pragma once

#include "ace/Svc_Handler.h" 
#include "ace/SOCK_Stream.h" 

class Client_Service : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> PARENT; 

public:
  Client_Service(void);
  virtual ~Client_Service(void);
  
  virtual int open(void* = 0); 
  virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  //virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE); 
  virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask); 
};

class Timer : public ACE_Task_Base
{
public:
    Timer (ACE_Thread_Manager *mgr = 0); 
    ~Timer (); 

    virtual int svc (); 
    virtual int handle_timeout (ACE_Time_Value const& time, void const* act); 

private: 
  int timer_; 
}; 
