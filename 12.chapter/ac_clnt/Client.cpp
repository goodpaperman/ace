#include "StdAfx.h"
#include "Client.h"
#include "ace/Reactor.h" 
#include "ace/os_ns_errno.h" 
#include "ace/os_ns_netdb.h" 
#include "../ac_react_srvr/DeviceCommandHeader.h" 

Client::Client(void)
: PARENT(0, 0, 0)
, iterations_(0) 
, notifier_(0, this, ACE_Event_Handler::WRITE_MASK) 
{
}

Client::~Client(void)
{
}

int Client::open(void* p /*=0*/)
{
  ACE_INET_Addr addr; 
  ACE_TCHAR addr_str[MAXHOSTNAMELEN] = { 0 }; 
  if(peer().get_remote_addr(addr) == -1 || 
    addr.addr_to_string(addr_str, MAXHOSTNAMELEN-1) == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connect with %s.\n"), addr_str)); 
  if(PARENT::open(p) == -1)
    return -1; 

  notifier_.reactor(this->reactor()); 
  msg_queue()->notification_strategy(&this->notifier_); 
  return this->reactor()->schedule_timer(this, 0, ACE_Time_Value::zero, ACE_Time_Value(2)); 
}

int Client::handle_input(ACE_HANDLE fd)
{
  char buf[64] = { 0 }; 
  ssize_t recv_cnt = peer().recv(buf, sizeof(buf)-1); 
  if(recv_cnt > 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%*C"), (int)recv_cnt, buf)); 
    return 0; 
  }

  if(recv_cnt == 0 || ACE_OS::last_error() != EWOULDBLOCK)
  {
    reactor()->end_reactor_event_loop(); 
    return -1; 
  }

  return 0; 
}

int Client::handle_timeout(ACE_Time_Value const& tv, void const* act /*= 0*/)
{
  if(iterations_++ >= ITERATIONS)
  {
    DeviceCommandHeader head; 
    head.device_id = -1; 
    head.length = 0; 

    ACE_Message_Block *mb = 0; 
    ACE_NEW_RETURN(mb, ACE_Message_Block(sizeof(head)), -1); 

    mb->copy((const char*)&head, sizeof(head)); 
    this->putq(mb); 
    //peer().close_writer(); 
    return 0; 
  }

  char msg[128] = { 0 }; 
  ACE_OS::sprintf(msg, "Command Sequence #%d\n", iterations_); 

  ACE_Message_Block* mb = 0; 
  ACE_NEW_RETURN(mb, ACE_Message_Block(128), -1); 

  DeviceCommandHeader head; 
  head.device_id = iterations_; 
  head.length = strlen(msg) + 1; 
  mb->copy((const char*)&head, sizeof(head)); 
  mb->copy(msg, head.length); 
  //ACE_OS::sprintf(mb->base(), "Iteration %d\n", iterations_); 
  //mb->wr_ptr(strlen(mb->base())); 
  this->putq(mb); 
  return 0; 
}

int Client::handle_output(ACE_HANDLE fd)
{
  ACE_Message_Block *mb = 0; 
  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
  while(this->getq(mb, &nowait) != -1)
  {
    ssize_t send_cnt = peer().send(mb->rd_ptr(), mb->length()); 
    if(send_cnt == -1)
    {
      this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK); 
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send")), 0); 
    }
    else 
      mb->rd_ptr((size_t)send_cnt); 

    if(mb->length() > 0)
    {
      this->ungetq(mb); 
      break; 
    }

    mb->release(); 
  }

  if(msg_queue()->is_empty())
    this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK); 
  else 
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK); 

  return 0; 
}