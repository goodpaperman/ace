#pragma once

#include "ace/Task.h" 

class react_task : public ACE_Task<ACE_SYNCH>
{
public:
  react_task(void);
  virtual ~react_task(void);

  virtual int svc(); 
};
