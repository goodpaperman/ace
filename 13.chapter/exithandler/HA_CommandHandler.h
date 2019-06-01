#pragma once
#include "ace/Task.h"

class ExitHandler : public ACE_At_Thread_Exit
{
public:
  virtual void apply(); 
}; 

class HA_CommandHandler : public ACE_Task_Base
{
public:
  HA_CommandHandler(ExitHandler& eh);
  virtual ~HA_CommandHandler(void);

  virtual int svc(); 

private:
  ExitHandler& eh_; 
};
