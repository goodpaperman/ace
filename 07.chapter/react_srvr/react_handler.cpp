#include "StdAfx.h"
#include "react_handler.h"
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 
#include "ace/Auto_Ptr.h" 

react_handler::react_handler(void)
{
}

react_handler::~react_handler(void)
{
}

int react_handler::open()
{
  ACE_INET_Addr peer_addr; 
  ACE_TCHAR peer_name[MAXHOSTNAMELEN] = { 0 }; 
  if(this->peer_.get_remote_addr(peer_addr) == 0 && 
    peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection from %s\n"), peer_name)); 

  int optval = 0; 
  int optlen = sizeof(optval); 
  if(peer_.get_option(SOL_SOCKET, SO_SNDBUF, (void *)&optval, &optlen) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send buf = %d, len = %d.\n"), optval, optlen)); 

  optval = 64; 
  optlen = sizeof optval; 
  if(peer_.set_option(SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("now = %d.\n"), optval)); 
  
  if(peer_.get_option(SOL_SOCKET, SO_RCVBUF, (void *)&optval, &optlen) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv buf = %d, len = %d.\n"), optval, optlen)); 

  optval = 64; 
  optlen = sizeof optval; 
  if(peer_.set_option(SOL_SOCKET, SO_RCVBUF, (void *)&optval, optlen) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("now = %d.\n"), optval)); 

  return this->reactor()->register_handler(this, ACE_Event_Handler::READ_MASK); 
}

int react_handler::handle_input(ACE_HANDLE handle /* =ACE_INVALID_HANDLE */)
{
  size_t const INPUT_SIZE = 4096000; 
  //char buffer[INPUT_SIZE] = { 0 }; 
  ACE_Auto_Array_Ptr<char> ptr(new char[INPUT_SIZE]()); 
  ssize_t recv_cnt(0), send_cnt(0); 

  if((recv_cnt = this->peer_.recv(ptr.get(), INPUT_SIZE-1)) <= 0)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Connection closed.\n")), -1); 
  }

  if(recv_cnt < INPUT_SIZE)
    ptr.get()[recv_cnt] = 0; 

  srand(ACE_OS::thr_self()); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv message: %s.\n"), ptr.get())); 
  while(recv_cnt < INPUT_SIZE-1)
    ptr.get()[recv_cnt++] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand()%52]; 

  ptr.get()[recv_cnt] = 0; 
  send_cnt = this->peer_.send(ptr.get(), (size_t)recv_cnt); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("send %d / %d.\n"), send_cnt, recv_cnt)); 
  if(send_cnt == recv_cnt)
    return 0; 

  //DebugBreak(); 
  if(send_cnt == -1 && ACE_OS::last_error() != EWOULDBLOCK)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send")), 0); 

  if(send_cnt == -1)
    send_cnt = 0; 

  ACE_Message_Block *mb = 0; 
  size_t remaining = (size_t)(recv_cnt - send_cnt); 
  ACE_NEW_RETURN(mb, ACE_Message_Block(ptr.get()+send_cnt, remaining), -1); 
  
  int output_off = this->queue_.is_empty(); 
  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
  if(this->queue_.enqueue_tail(mb, &nowait) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t)  %p; discarding data\n"), ACE_TEXT("enqueue failed"))); 
    mb->release(); 
    return 0; 
  }

  if(output_off)
    return this->reactor()->register_handler(this, WRITE_MASK); 

  return 0; 
}

int react_handler::handle_output(ACE_HANDLE handle /* =ACE_INVALID_HANDLE */)
{
  ACE_Message_Block *mb = 0; 
  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
  while(this->queue_.dequeue_head(mb, &nowait) == 0)
  {
    ssize_t send_cnt = this->peer_.send(mb->rd_ptr(), mb->length()); 
    if(send_cnt == -1)
      ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send"))); 
    else 
      mb->rd_ptr((size_t)send_cnt); 

    if(mb->length() > 0)
    {
      this->queue_.enqueue_head(mb); 
      break; 
    }

    mb->release(); 
  }

  return this->queue_.is_empty() ? -1 : 0; 
}

int react_handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
  if(close_mask == WRITE_MASK)
    return 0; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) handle_close\n"))); 
  //close_mask = ALL_EVENTS_MASK; // | DONT_CALL; 
  this->reactor()->remove_handler(this, ALL_EVENTS_MASK | DONT_CALL); 
  //this->reactor()->end_reactor_event_loop(); 
  this->peer_.close(); 
  this->queue_.close(); 
  delete this; 
  //abort(); 

  raise(time(0) % 2 == 0 ? SIGABRT : SIGTERM); 
  return 0; 
}