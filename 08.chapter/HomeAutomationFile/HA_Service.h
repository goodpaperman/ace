#pragma once

#include "ace/Asynch_IO.h"

class HA_Service : public ACE_Service_Handler
{
public:
  HA_Service(void);
  virtual ~HA_Service(void);
  virtual void addresses(const ACE_INET_Addr& remote_address, const ACE_INET_Addr& local_address); 
  virtual void open(ACE_HANDLE new_handle, ACE_Message_Block &message_block); 
  virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result &result); 
  virtual void handle_write_file(const ACE_Asynch_Write_File::Result &result); 

private:
  off_t header_; 
  ACE_UINT64 offset_; 
  ACE_HANDLE outfile_; 
  ACE_Asynch_Read_Stream reader_; 
  ACE_Asynch_Write_File writer_; 
};
