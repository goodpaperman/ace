#pragma once

#include "ace/Task.h" 

class acceptor_task : public ACE_Task<ACE_SYNCH>
{
public:
  acceptor_task(void);
  virtual ~acceptor_task(void);

  virtual int svc(); 
};
