#pragma once

#include "ace/Singleton.h" 

class none_service
{
public:
    none_service(void);
    ~none_service(void);
};


//typedef ACE_Singleton <none_service, ACE_Thread_Mutex> NONE_SVC; 
typedef ACE_Unmanaged_Singleton <none_service, ACE_Thread_Mutex> NONE_SVC; 