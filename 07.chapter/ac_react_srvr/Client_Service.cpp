#include "StdAfx.h"
#include "Client_Service.h"
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 

Client_Service::Client_Service(void)
// to prevent create the default reactor instance.
: PARENT(0, 0, 0) 
{
}

Client_Service::~Client_Service(void)
{
}

int Client_Service::open(void* p /*= 0*/)
{
  if(PARENT::open(p) == -1)
    return -1; 

  ACE_INET_Addr peer_addr; 
  ACE_TCHAR peer_name[MAXHOSTNAMELEN] = { 0 }; 
  if(peer().get_remote_addr(peer_addr) == 0 && 
    peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection from %s\n"), peer_name)); 

  return 0; 
}

int Client_Service::handle_input(ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/)
{
  size_t const INPUT_SIZE = 4096; 
  char buffer[INPUT_SIZE] = { 0 }; 
  //ACE_Auto_Array_Ptr<char> ptr(new char[INPUT_SIZE]()); 
  ssize_t recv_cnt(0), send_cnt(0); 

  recv_cnt = this->peer().recv(buffer, INPUT_SIZE-1); 
  if(recv_cnt <= 0)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Connection closed.\n")), -1); 
  }

  if(recv_cnt < INPUT_SIZE)
    buffer[recv_cnt] = 0; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv message: %s.\n"), buffer)); 
  send_cnt = this->peer().send(buffer, (size_t)recv_cnt); 
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
  ACE_NEW_RETURN(mb, ACE_Message_Block(buffer+send_cnt, remaining), -1); 
  
  int output_off = this->msg_queue()->is_empty(); 
  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
  if(this->putq(mb, &nowait) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t)  %p; discarding data\n"), ACE_TEXT("enqueue failed"))); 
    mb->release(); 
    return 0; 
  }

  if(output_off)
    return this->reactor()->register_handler(this, WRITE_MASK); 

  return 0; 
}

int Client_Service::handle_output(ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/)
{
  ACE_Message_Block *mb = 0; 
  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
  while(this->getq(mb, &nowait) == 0)
  {
    ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length()); 
    if(send_cnt == -1)
      ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send"))); 
    else 
      mb->rd_ptr((size_t)send_cnt); 

    if(mb->length() > 0)
    {
      this->ungetq(mb); 
      break; 
    }

    mb->release(); 
  }

  return this->msg_queue()->is_empty() ? -1 : 0; 
}

int Client_Service::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
  if(mask == WRITE_MASK)
    return 0; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) handle_close\n"))); 
  return PARENT::handle_close(fd, mask); 
}


#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>; 
#endif 