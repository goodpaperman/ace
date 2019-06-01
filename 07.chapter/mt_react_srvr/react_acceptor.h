#pragma once

#include "ace/Log_Msg.h" 
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Acceptor.h" 
#include "ace/Reactor.h" 

class react_acceptor : public ACE_Event_Handler
{
public:
  react_acceptor(void);
  virtual ~react_acceptor(void);

  int open(const ACE_INET_Addr &listen_addr); 
  virtual ACE_HANDLE get_handle() const { return acceptor_.get_handle(); }
  virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE); 
  virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask); 

protected:
  ACE_SOCK_Acceptor acceptor_; 
};
