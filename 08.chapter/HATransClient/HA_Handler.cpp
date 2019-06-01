#include "StdAfx.h"
#include "HA_Handler.h"
#include "ace/Log_Msg.h" 
#include "ace/Message_Block.h" 
#include "ace/INET_Addr.h" 
#include "ace/os_ns_sys_socket.h" 
#include "ace/os_memory.h" 
#include "ace/File_Connector.h" 
//#include "ace/os_ns_unistd.h" 

HA_Handler::HA_Handler(void)
: ACE_Service_Handler()
, trans_()
{
}

HA_Handler::~HA_Handler(void)
{
  if(this->handle() != ACE_INVALID_HANDLE)
    ACE_OS::closesocket(this->handle()); 
}

#define BUFSIZE 64
void HA_Handler::addresses(const ACE_INET_Addr& remote, const ACE_INET_Addr& local)
{
  char addrstr1[BUFSIZE] = { 0 }, addrstr2[BUFSIZE] = { 0 }; 
  if(remote.addr_to_string(addrstr1, BUFSIZE-1) == 0 && 
    local.addr_to_string(addrstr2, BUFSIZE-1) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("validate_connection: remote %s, local %s\n"), addrstr1, addrstr2)); 
}

void HA_Handler::open(ACE_HANDLE new_handle, ACE_Message_Block& message_block)
{
  this->handle(new_handle); 
  if(this->trans_.open(*this) != 0)
  {
    delete this; 
    return; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connection opened.\n"))); 
  ACE_FILE_IO file; 
  ACE_FILE_Addr filename("e:\\books\\jjhou.rar"); 
  ACE_FILE_Connector conn; 
  if(conn.connect(file, filename, 0, ACE_Addr::sap_any, 0, _O_RDONLY | _O_EXCL | FILE_FLAG_SEQUENTIAL_SCAN) != 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open file"))); 
    delete this; 
    return; 
  }

  ACE_FILE_Info finfo = { 0 }; 
  file.get_info(finfo); 

  ACE_Message_Block* head_mb = 0; 
  ACE_NEW_NORETURN(head_mb, ACE_Message_Block(sizeof(off_t))); 
  head_mb->copy((char*)&finfo.size_, sizeof(off_t)); 

  ACE_Asynch_Transmit_File::Header_And_Trailer *header = 0; 
  ACE_NEW_NORETURN(header, ACE_Asynch_Transmit_File::Header_And_Trailer(head_mb, head_mb->length())); 
  if(this->trans_.transmit_file(file.get_handle(), header, 0, 0, 0, ACE_OS::getpagesize() * 64) != 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("HA_Handler begin transmit file"))); 
    file.close(); 
    head_mb->release(); 
    delete header; 
    delete this; 
    return; 
  }
}

void HA_Handler::handle_transmit_file(const ACE_Asynch_Transmit_File::Result& result)
{
  if(!result.success() || result.bytes_transferred() == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("transmit failed, error = %d.\n"), result.error())); 
  }
  else
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("transmit over, transfer = %d, socket = %d.\n"), 
      result.bytes_transferred(), 
      result.socket())); 
  }
  
  ACE_OS::close(result.file()); 
  result.header_and_trailer()->header()->release(); 
  delete result.header_and_trailer(); 
  delete this; 
}
