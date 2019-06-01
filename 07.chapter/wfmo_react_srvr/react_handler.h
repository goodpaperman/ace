#pragma once

#include "ace/Reactor.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/Null_Mutex.h" 
#include "ace/Null_Condition.h" 

class react_handler : public ACE_Event_Handler
{
public:
  react_handler(void);
  virtual ~react_handler(void);

  int open(); 
  ACE_SOCK_Stream& peer() const { return const_cast<ACE_SOCK_Stream&>(peer_); } 
  virtual ACE_HANDLE get_handle() const { return this->peer_.get_handle(); } 
  virtual int handle_input(ACE_HANDLE handle = INVALID_HANDLE_VALUE); 
  virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask); 

protected:
  ACE_SOCK_Stream peer_; 
  static int curr_prio_; 
};
