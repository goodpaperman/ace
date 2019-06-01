#pragma once

#include "ace/Local_Tokens.h" 

#define USE_SAME_TOKEN

class HA_Device_Repository
{
public:
    enum { N_DEVICES = 50 }; 
    HA_Device_Repository(void);
    ~HA_Device_Repository(void);

    int update_device(int device_id, char *commands); 

private:
    ACE_Local_Mutex *tokens_[N_DEVICES]; 
};
