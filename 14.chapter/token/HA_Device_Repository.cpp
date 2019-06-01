#include "StdAfx.h"
#include "HA_Device_Repository.h"
#include "ace/OS_NS_unistd.h" 

HA_Device_Repository::HA_Device_Repository(void)
{
    for(int i=0; i<N_DEVICES; ++ i)
    {
        tokens_[i] = new ACE_Local_Mutex(
#if defined(USE_SAME_TOKEN)
            "same_token", 
#else 
            0, 
#endif 
            0, 1); 
    }
}

HA_Device_Repository::~HA_Device_Repository(void)
{
    for(int i=0; i<N_DEVICES; ++ i)
        delete tokens_[i]; 
}

int HA_Device_Repository::update_device(int device_id, char *commands)
{
    this->tokens_[device_id]->acquire(); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) update device %u.\n"), device_id)); 
    ACE_OS::sleep(ACE_Time_Value(0, 100000)); 
    this->tokens_[device_id]->release(); 
    return 0; 
}
