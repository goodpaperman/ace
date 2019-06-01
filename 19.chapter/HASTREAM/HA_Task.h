#pragma once

#include "ace/Service_Config.h" 

class HA_Task : public ACE_Task <ACE_MT_SYNCH>
{
public:

    HA_Task(void)
    {
    }

    virtual ~HA_Task(void)
    {
    }

    virtual int open (void *args = 0)
    {
        return activate (); 
    }

    virtual int suspend (void)
    {
        ACE_DEBUG ((LM_DEBUG, "suspend task\n")); 
        return ACE_Task <ACE_MT_SYNCH>::suspend (); 
    }

    virtual int resume (void)
    {
        ACE_DEBUG ((LM_DEBUG, "resume task\n")); 
        return ACE_Task <ACE_MT_SYNCH>::resume (); 
    }
    virtual int put (ACE_Message_Block *mb, ACE_Time_Value *tv = 0)
    {
        return putq (mb, tv); 
    }
};


#define HA_Stream_Export _declspec (dllexport)