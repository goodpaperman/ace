#pragma once

#include "ace/Asynch_IO.h"
#include "ace/INET_Addr.h" 
#include "ace/FILE_IO.h" 

class HA_Handler : public ACE_Service_Handler
{
public:
  HA_Handler(void);
  virtual ~HA_Handler(void);  
  virtual void addresses(const ACE_INET_Addr& remote_address, const ACE_INET_Addr& local_address); 
  virtual void open(ACE_HANDLE new_handle, ACE_Message_Block &message_block); 
  virtual void handle_read_dgram(const ACE_Asynch_Read_Dgram::Result& result); 
  virtual void handle_write_dgram(const ACE_Asynch_Write_Dgram::Result& result); 

private:
  off_t filesize_; 
  ACE_FILE_IO file_; 
  ACE_Asynch_Read_Dgram reader_; 
  ACE_Asynch_Write_Dgram writer_; 
};
