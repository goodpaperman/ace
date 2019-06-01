#pragma once
#include "ace/Task.h" 
#include "HA_Device_Repository.h" 

class HA_CommandHandler : public ACE_Task_Base
{
public:
    enum { N_THREADS = 5 }; 
    HA_CommandHandler(HA_Device_Repository &rep);
    ~HA_CommandHandler(void);

    virtual int svc(); 

private:
    HA_Device_Repository &rep_; 
};
