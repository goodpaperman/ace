#include "StdAfx.h"
#include "Message_Receiver.h"
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 

Message_Receiver::Message_Receiver(HA_CommandHandler* handler)
: PARENT(0, 0, 0)
, handler_(handler)
{
}

Message_Receiver::~Message_Receiver(void)
{
}

int Message_Receiver::open(void* p)
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

int Message_Receiver::read_header(DeviceCommandHeader *head)
{
  if(peer().recv_n(head, sizeof(DeviceCommandHeader)) != sizeof(DeviceCommandHeader))
    return -1; 
  else 
    return 0; 
}

int Message_Receiver::copy_payload(ACE_Message_Block* mb, int length)
{
  int result = this->peer().recv_n(mb->wr_ptr(), length); 
  if(result <= 0)
  {
    mb->release(); 
    return result; 
  }

  mb->wr_ptr(length); 
  return 0; 
}

ACE_Message_Block* Message_Receiver::shut_down_message()
{
  ACE_Message_Block *mb = 0; 
  ACE_NEW_RETURN(mb, ACE_Message_Block(0, ACE_Message_Block::MB_HANGUP), 0); 
  return mb; 
}

int Message_Receiver::handle_input(ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/)
{
  DeviceCommandHeader dch; 
  if(this->read_header(&dch) < 0)
    return -1; 

  if(dch.device_id < 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("receive shut down message.\n"))); 
    this->handler_->putq(shut_down_message()); 
    return -1; 
  }

  ACE_Message_Block *mb = 0; 
  ACE_NEW_RETURN(mb, ACE_Message_Block(dch.length + sizeof(dch)), -1); 
  mb->copy((const char*)&dch, sizeof dch); 
  if(this->copy_payload(mb, dch.length) < 0)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("Receive Failure")), -1); 

  this->handler_->putq(mb); 
  return 0; 
}

int Message_Receiver::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) handle_close\n"))); 
  return PARENT::handle_close(fd, mask); 
}

