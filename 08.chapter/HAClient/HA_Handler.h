#pragma once

#include "ace/Asynch_IO.h"

class HA_Handler : public ACE_Service_Handler
{
public:
  HA_Handler(void);
  virtual ~HA_Handler(void);  
  virtual void addresses(const ACE_INET_Addr& remote_address, const ACE_INET_Addr& local_address); 
  virtual void open(ACE_HANDLE new_handle, ACE_Message_Block &message_block); 
  virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result &result); 
  virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result &result); 

private:
  ACE_Asynch_Read_Stream reader_; 
  ACE_Asynch_Write_Stream writer_; 
};
