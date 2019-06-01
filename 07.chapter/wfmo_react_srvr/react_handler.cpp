#include "StdAfx.h"
#include "react_handler.h"
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 
#include "ace/Auto_Ptr.h" 

int react_handler::curr_prio_ = ACE_Event_Handler::LO_PRIORITY; 

react_handler::react_handler(void)
{
}

react_handler::~react_handler(void)
{
}

int react_handler::open()
{
  this->priority(curr_prio_++); 

  ACE_INET_Addr peer_addr; 
  ACE_TCHAR peer_name[MAXHOSTNAMELEN] = { 0 }; 
  if(this->peer_.get_remote_addr(peer_addr) == 0 && 
    peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t|%d|%d) Connection from %s\n"), get_handle(), priority(), peer_name)); 

  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P | %t | %d | %d)\n"), get_handle(), priority())); 
  return this->reactor()->register_handler(this, ACE_Event_Handler::READ_MASK); 
}

int react_handler::handle_input(ACE_HANDLE handle /* =ACE_INVALID_HANDLE */)
{
  size_t const INPUT_SIZE = 4096; 
  //char buffer[INPUT_SIZE] = { 0 }; 
  ACE_Auto_Array_Ptr<char> ptr(new char[INPUT_SIZE]()); 
  ssize_t recv_cnt(0), send_cnt(0); 

  if((recv_cnt = this->peer_.recv(ptr.get(), INPUT_SIZE-1)) <= 0)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P | %t | %d | %d) Connection closed.\n"), get_handle(), priority()), -1); 
  }

  if(recv_cnt < INPUT_SIZE)
    ptr.get()[recv_cnt] = 0; 

  int i = recv_cnt; 
  srand(ACE_OS::thr_self()); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P | %t | %d | %d) recv message: %s.\n"), get_handle(), priority(), ptr.get())); 
  while(i > 0)
    ptr.get()[--i] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand()%52]; 

  send_cnt = this->peer_.send(ptr.get(), (size_t)recv_cnt); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("send %d / %d.\n"), send_cnt, recv_cnt)); 
  if(send_cnt == recv_cnt)
    return 0; 

  return -1; 
}


int react_handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P | %t | %d | %d) handle_close\n"), get_handle(), priority())); 
  //close_mask = ALL_EVENTS_MASK; // | DONT_CALL; 
  this->reactor()->remove_handler(this, ALL_EVENTS_MASK | DONT_CALL); 
  //this->reactor()->end_reactor_event_loop(); 
  this->peer_.close(); 
  delete this; 
  return 0; 
}