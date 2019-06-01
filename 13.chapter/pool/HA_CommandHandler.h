#pragma once
#include "ace/Task_T.h"

class HA_CommandHandler : public ACE_Task<ACE_MT_SYNCH>
{
public:
  HA_CommandHandler(char const* name);
  virtual ~HA_CommandHandler(void);

  virtual int svc(); 

private:
  char const* name_; 
};
