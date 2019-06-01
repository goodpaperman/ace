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
, filesize_(0)
, file_()
, writer_()
{
}

HA_Handler::~HA_Handler(void)
{
  file_.close(); 
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
  filesize_ = 0; 
  this->handle(new_handle); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connection opened.\n"))); 
  ACE_FILE_Addr filename("e:\\books\\jjhou.rar"); 
  ACE_FILE_Connector conn; 
  if(conn.connect(file_, filename, 0, ACE_Addr::sap_any, 0, _O_RDONLY | _O_EXCL | FILE_FLAG_SEQUENTIAL_SCAN/*| FILE_FLAG_OVERLAPPED*/) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open file"))); 
    delete this; 
    return; 
  }

  ACE_FILE_Info finfo = { 0 }; 
  file_.get_info(finfo); 

  if(this->writer_.open(*this) != 0)
  {
    delete this; 
    return; 
  }

  ACE_Message_Block* mb = 0; 
  ACE_NEW_NORETURN(mb, ACE_Message_Block(ACE_OS::getpagesize() * 64)); 
  mb->copy((char*)&finfo.size_, sizeof(off_t)); 
  if(this->writer_.write(*mb, sizeof(off_t)) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("HA_Handler send file size"))); 
    mb->release(); 
    delete this; 
    return; 
  }

  filesize_ = finfo.size_; 
}

void HA_Handler::handle_write_stream(const ACE_Asynch_Write_Stream::Result& result)
{
  ACE_Message_Block& mb = result.message_block(); 
  if(!result.success() || result.bytes_transferred() == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send failed, error = %d.\n"), result.error())); 
    mb.release(); 
    delete this; 
    return; 
  }
  else
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send %8.3u\n"), result.bytes_transferred())); 

    mb.reset(); 
    size_t read = file_.recv(mb.rd_ptr(), mb.space()); 
    if(read == -1)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p"), ACE_TEXT("read file"))); 
      mb.release(); 
      delete this; 
      return; 
    }
    else if(read == 0)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("reach file end"))); 
      mb.release(); 
      delete this; 
      return; 
    }

    mb.wr_ptr(read); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("read %8.3u = %3.2f%%.\n"), 
      mb.length(), file_.tell()*100.0/filesize_)); 

    if(this->writer_.write(mb, mb.length()) == -1)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p"), ACE_TEXT("send"))); 
      mb.release(); 
      delete this; 
      return; 
    }

  }
}
