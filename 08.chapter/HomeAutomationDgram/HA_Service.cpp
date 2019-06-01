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
, offset_(0) 
, file_()
, reader_()
, writer_()
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
  offset_ = 0; 
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

  if(this->reader_.open(*this) != 0 ||
    this->writer_.open(*this) != 0)
  {
    file_.close(); 
    delete this; 
    return; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("service opened.\n"))); 

  size_t read = 0; 
  ACE_Message_Block *mb = 0; 
  ACE_NEW_NORETURN(mb, ACE_Message_Block(ACE_OS::getpagesize() * 8)); 
  if(this->reader_.recv(mb, read, 0) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("HA_Service begin read"))); 
    mb->release(); 
    file_.close(); 
    delete this; 
    return; 
  }
}

void HA_Service::handle_read_dgram(const ACE_Asynch_Read_Dgram::Result &result)
{
  ACE_Message_Block& mb = *result.message_block(); 
  if(!result.success() || result.bytes_transferred() == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("connection break.\n"))); 
    mb.release(); 
    delete this; 
  }
  else 
  {
    size_t send = 0; 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv %8.3u\n"), result.bytes_transferred())); 
    if(filesize_ == 0)
    {
      ACE_ASSERT(mb.length() == sizeof(off_t)); 
      filesize_ = *(off_t*)mb.rd_ptr(); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("file size = %u\n"), filesize_)); 
      //ACE_OS::ftruncate(outfile_, header_); 
      file_.truncate(filesize_); 
      // reposition the file pointer to header for writing data.
      file_.seek(0, SEEK_SET); 
    }
    else 
    {
      //ACE_OS::write(outfile_, mb.rd_ptr(), mb.length()); 
      file_.send(mb.rd_ptr(), mb.length()); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("write %8.3u = %3.2f%%.\n"), mb.length(), file_.tell()*100.0/filesize_)); 

      mb.reset(); 
      mb.copy((char*)&send, sizeof(send)); 
    }
    
    // send the reply.
    //mb.reset(); 
    ACE_INET_Addr addr; 
    result.remote_address(addr); 
    this->writer_.send(&mb, send, 0, addr); 
  }
}

void HA_Service::handle_write_dgram(const ACE_Asynch_Write_Dgram::Result& result)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("send reply %u.\n"), result.bytes_transferred())); 
  ACE_Message_Block& mb = *result.message_block(); 
  if(!result.success() || result.bytes_transferred() == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("connection break.\n"))); 
    mb.release(); 
    delete this; 
  }
  else
  {
    if(file_.tell() >= filesize_)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("transmit over.\n"))); 
      mb.release(); 
      delete this; 
      return; 
    }

    mb.reset(); 
    size_t read = 0; 
    this->reader_.recv(&mb, read, 0); 
  }
}
