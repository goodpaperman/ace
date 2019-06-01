#include "StdAfx.h"
#include "react_acceptor.h"
#include "react_handler.h" 
#include "ace/Auto_Ptr.h" 

react_acceptor::react_acceptor(void)
{
}

react_acceptor::~react_acceptor(void)
{
  //handle_close(ACE_INVALID_HANDLE, 0); 
}

int react_acceptor::open(ACE_INET_Addr const& listen_addr)
{
  if(this->acceptor_.open(listen_addr, 1) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open acceptor")), -1); 

  return this->reactor()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK); 
}

int react_acceptor::handle_input(ACE_HANDLE handle /* =ACE_INVALID_HANDLE */)
{
  react_handler *handler = 0; 
  ACE_NEW_RETURN(handler, react_handler(), -1); 
  ACE_Auto_Basic_Ptr<react_handler> ptr(handler); 
  if(this->acceptor_.accept(handler->peer()) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("accept")), -1); 

  ptr.release(); 
  handler->reactor(this->reactor()); 
  if(handler->open() == -1)
    handler->handle_close(ACE_INVALID_HANDLE, ACE_Event_Handler::ALL_EVENTS_MASK); 
  
  return 0; 
}

int react_acceptor::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
  if(this->acceptor_.get_handle() != INVALID_HANDLE_VALUE)
  {
    this->reactor()->remove_handler(this, ACCEPT_MASK | DONT_CALL); 
    this->acceptor_.close(); 
    delete this; 
  }

  return 0; 
}