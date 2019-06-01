#include "StdAfx.h"
#include "HA_Service.h"
#include "ace/Log_Msg.h" 
#include "ace/Message_Block.h" 
#include "ace/INET_Addr.h" 
#include "ace/os_ns_sys_socket.h" 
#include "ace/os_ns_unistd.h" 
#include "ace/os_memory.h" 
#include "ace/File_Connector.h" 


HA_Service::HA_Service(void)
: ACE_Service_Handler()
, filesize_(0) 
, file_()
, reader_()
{
}

HA_Service::~HA_Service(void)
{
  file_.close(); 
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
  filesize_ = 0; 
  this->handle(new_handle); 

  ACE_FILE_Addr filename("jjhou.rar"); // =(ACE_FILE_Addr&)ACE_Addr::sap_any; 
  ACE_FILE_Connector conn; 
  if(conn.connect(file_, filename, 0, ACE_Addr::sap_any, 
    0, _O_WRONLY | _O_CREAT | _O_TRUNC /*| FILE_FLAG_OVERLAPPED*/) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p"), ACE_TEXT("open output file"))); 
    delete this; 
    return; 
  }

  if(this->reader_.open(*this) != 0)
  {
    delete this; 
    return; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("service opened.\n"))); 
  ACE_Message_Block *mb = 0; 
  ACE_NEW_NORETURN(mb, ACE_Message_Block(ACE_OS::getpagesize() * 64)); 
  if(this->reader_.read(*mb, sizeof(off_t)) == -1)
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
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv  %8.3u\n"), result.bytes_transferred())); 
    if(filesize_ == 0)
    {
      ACE_ASSERT(mb.length() == sizeof(off_t)); 
      filesize_ = *(off_t*)mb.rd_ptr(); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("file size = %u\n"), filesize_)); 
      //ACE_OS::ftruncate(outfile_, header_); 
      file_.truncate(filesize_); 
      file_.seek(0, SEEK_SET); 
    }
    else 
    {
      if(file_.send(mb.rd_ptr(), mb.length()) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("write file"))); 
        mb.release(); 
        delete this; 
        return; 
      }
      
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("write %8.3u = %3.2f%%.\n"), mb.length(), file_.tell()*100.0/filesize_)); 
    }

    mb.reset(); 
    if(this->reader_.read(mb, mb.space()) == -1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("HA_Service begin read"))); 
      mb.release(); 
      delete this; 
      return; 
    }
  }
}
