#pragma once

#include "ace/Asynch_Acceptor.h" 
#include "HA_Service.h" 

class HA_Acceptor : public ACE_Asynch_Acceptor<HA_Service>
{
public:
  HA_Acceptor(void);
  virtual ~HA_Acceptor(void);

  virtual int validate_connection(
    const ACE_Asynch_Accept::Result& result, 
    const ACE_INET_Addr& remote, 
    const ACE_INET_Addr& local); 
};
