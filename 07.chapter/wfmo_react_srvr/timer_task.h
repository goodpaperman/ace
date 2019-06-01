#pragma once

#include "ace/Task.h" 

class timer_task : public ACE_Task<ACE_NULL_SYNCH> 
{
public:
  timer_task(void);
  virtual ~timer_task(void);

  virtual int svc(); 
  virtual int handle_timeout(ACE_Time_Value const& tv, void const* act = 0); 
};
