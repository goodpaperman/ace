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
, header_(0) 
, offset_(0) 
, outfile_(ACE_INVALID_HANDLE)
, reader_()
, writer_()
{
}

HA_Service::~HA_Service(void)
{
  if(outfile_ != ACE_INVALID_HANDLE)
  {
    ACE_OS::close(outfile_); 
    outfile_ = ACE_INVALID_HANDLE; 
  }

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
  header_ = 0; 
  offset_ = 0; 
  outfile_ = ACE_INVALID_HANDLE; 
  this->handle(new_handle); 
  ACE_FILE_IO file; 
  ACE_FILE_Addr filename("jjhou.rar"); // =(ACE_FILE_Addr&)ACE_Addr::sap_any; 
  ACE_FILE_Connector conn; 
  if(conn.connect(file, filename, 0, ACE_Addr::sap_any, 
    0, _O_WRONLY | _O_CREAT | _O_TRUNC | FILE_FLAG_OVERLAPPED) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%p"), ACE_TEXT("open output file"))); 
    delete this; 
    return; 
  }

  if(this->reader_.open(*this) != 0 ||
    this->writer_.open(*this, file.get_handle()) != 0)
  {
    file.close(); 
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
    file.close(); 
    delete this; 
    return; 
  }

  outfile_ = file.get_handle(); 
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
  //else if(result.bytes_transferred() < result.bytes_to_read())
  //{
  //  if(reader_.read(mb, result.bytes_to_read() - result.bytes_transferred()) == -1)
  //  {
  //    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("continue read"))); 
  //    mb.release(); 
  //    ACE_OS::close(result.handle()); 
  //    delete this; 
  //  }
  //}
  else 
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv %8.3u\n"), result.bytes_transferred())); 
    if(header_ == 0)
    {
      ACE_ASSERT(mb.length() == sizeof(off_t)); 
      header_ = *(off_t*)mb.rd_ptr(); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("file size = %u\n"), header_)); 
      ACE_OS::ftruncate(outfile_, header_); 
      
      mb.reset(); 
      this->reader_.read(mb, mb.space()); 
      //ACE_DEBUG((LM_DEBUG, ACE_TEXT("start recv %d.\n"), mb.space())); 
    }
    else 
    {
      if(this->writer_.write(mb, mb.length(), (u_long)((offset_ << 32) >> 32), (u_long)(offset_ >> 32)) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("starting write"))); 
        mb.release(); 
        delete this; 
        return; 
      }

      //ACE_DEBUG((LM_DEBUG, ACE_TEXT("start send %d.\n"), mb.length())); 
    }
  }
}

void HA_Service::handle_write_file(const ACE_Asynch_Write_File::Result &result)
{
  offset_ += result.bytes_transferred(); 
  ACE_Message_Block& mb = result.message_block(); 
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("send %d.\n"), result.bytes_transferred())); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("write %8.3u = %3.2f%%.\n"), result.bytes_transferred(), offset_*100.0/header_)); 
  //if(result.bytes_transferred() < result.bytes_to_write())
  //{
  //  if(this->writer_.write(mb, result.bytes_to_write() - result.bytes_transferred(), 
  //    (u_long)((offset_ << 32) >> 32), (u_long)(offset_ >> 32)) == -1)
  //  {
  //    ACE_ERROR((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("continue write"))); 
  //    mb.release(); 
  //    ACE_OS::close(result.handle()); 
  //    delete this; 
  //  }

  //  return; 
  //}

  mb.reset(); 
  this->reader_.read(mb, mb.space()); 
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("start recv %d.\n"), mb.space())); 
}