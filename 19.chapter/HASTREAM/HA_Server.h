#pragma once

#include "ace/Module.h" 
#include "HA_Task.h" 

class HA_Server_Convert : public HA_Task
{
public:
    virtual int open (void *args = 0)
    {
        // don't activate threads.
        return ACE_Task <ACE_MT_SYNCH> ::open (args); 
    }

    int svc () 
    {
        ACE_DEBUG ((LM_DEBUG, "HA_Server_Convert startup\n")); 
        ACE_DEBUG ((LM_DEBUG, "HA_Server_Convert shutdown\n")); 
        return 0; 
    }

    virtual int fini ()
    {
        ACE_DEBUG ((LM_DEBUG, "HA_Server fini\n")); 
        return 0; 
    }

    virtual int put (ACE_Message_Block *mb, ACE_Time_Value *tv = 0)
    {
        char buf[4096] = { 0 }; 
        ACE_DEBUG ((LM_DEBUG, "[display] %s", mb->rd_ptr ())); 
        ACE_OS::sprintf (buf, "echo: %s", mb->rd_ptr ()); 
        mb->release (); 

        // copy is really cat
        mb = new ACE_Message_Block (4096); 
        mb->copy (buf, strlen (buf)+1); 
        ACE_DEBUG ((LM_DEBUG, "[convert] %s", mb->rd_ptr ())); 
        return reply (mb, tv); 
    }
}; 

class HA_Server : public ACE_Module <ACE_MT_SYNCH>
{
public:
    HA_Server(void);
    ~HA_Server(void);
};
