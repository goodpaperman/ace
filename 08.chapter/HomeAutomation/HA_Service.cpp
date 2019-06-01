#include "StdAfx.h"
#include "HA_Service.h"
#include "ace/Log_Msg.h" 
#include "ace/Message_Block.h" 
#include "ace/INET_Addr.h" 
#include "ace/os_ns_sys_socket.h" 
#include "ace/os_memory.h" 


HA_Service::HA_Service(void)
: ACE_Service_Handler()
, reader_()
, writer_()
{
}

HA_Service::~HA_Service(void)
{
  if(this->handle() != ACE_INVALID_HANDLE)
    ACE_OS::closesocket(this->handle()); 
}

#define BUFSIZE 64
void HA_Service::addresses(const ACE_INET_Addr& remote, const ACE_INET_Addr& local)
{
  char addrstr1[BUFSIZE] = { 0 }, addrstr2[BUFSIZE] = { 0 }; 
  if(remote.addr_to_string(addrstr1, BUFSIZE-1) == 0 && 
    local.addr_to_string(addrstr2, BUFSIZE-1) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("addresses: remote %s, local %s\n"), addrstr1, addrstr2)); 
}

void HA_Service::open(ACE_HANDLE new_handle, ACE_Message_Block& message_block)
{
  this->handle(new_handle); 
  if(this->reader_.open(*this) != 0 ||
    this->writer_.open(*this) != 0)
  {
    delete this; 
    return; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("service opened.\n"))); 
  ACE_Message_Block *mb = 0; 
  ACE_NEW_NORETURN(mb, ACE_Message_Block(1024)); 
  if(this->reader_.read(*mb, mb->space()) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("HA_Service begin read"))); 
    mb->release(); 
    delete this; 
    return; 
  }
}

void HA_Service::handle_read_stream(const ACE_Asynch_Read_Stream::Result &result)
{
  ACE_Message_Block& mb = result.message_block(); 
  if(!result.success() || result.bytes_transferred() == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("connection break.\n"))); 
    mb.release(); 
    delete this; 
  }
  else
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv %d : %s\n"), result.bytes_transferred(), mb.rd_ptr())); 
    if(this->writer_.write(mb, mb.length()) == -1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("starting write"))); 
      delete this; 
    }
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("start send %d.\n"), mb.length())); 
  }
}

void HA_Service::handle_write_stream(const ACE_Asynch_Write_Stream::Result &result)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("send %d.\n"), result.bytes_transferred())); 
  ACE_Message_Block& mb = result.message_block(); 
  mb.reset(); 

  this->reader_.read(mb, mb.space()); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("start recv %d.\n"), mb.space())); 
}