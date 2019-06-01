#pragma once

#include "ace/Task.h" 

class event_task : public ACE_Task<ACE_NULL_SYNCH> 
{
public:
  event_task(void);
  virtual ~event_task(void);

  virtual int svc(); 
};
