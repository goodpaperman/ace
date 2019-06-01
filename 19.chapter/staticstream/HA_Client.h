#pragma once

#include "ace/Module.h" 
#include "HA_Task.h" 


class HA_Client_Driver : public HA_Task
{
public:
    virtual int open (void *args = 0)
    {
        ACE_DEBUG ((LM_DEBUG, "HA_Client_Driver open\n")); 
        ACE_Event_Handler::register_stdin_handler (this, ACE_Reactor::instance (), ACE_Thread_Manager::instance ()); 
        // don't activate threads.
        return ACE_Task <ACE_MT_SYNCH>::open (args); 
    }

    int svc () 
    {
        ACE_DEBUG ((LM_DEBUG, "HA_Client_Driver startup\n")); 
        ACE_DEBUG ((LM_DEBUG, "HA_Client_Driver shutdown\n")); 
        return 0; 
    }

    virtual int fini ()
    {
        ACE_DEBUG ((LM_DEBUG, "HA_Client fini\n")); 
        ACE_Event_Handler::remove_stdin_handler (ACE_Reactor::instance (), ACE_Thread_Manager::instance ()); 
        return 0; 
    }

    virtual int put (ACE_Message_Block *mb, ACE_Time_Value *tv = 0)
    {
        ACE_DEBUG ((LM_DEBUG, "[display] %s\n", mb->rd_ptr ())); 
        mb->release (); 
        return 0; 
    }

    virtual int handle_input (ACE_HANDLE h)
    {
        ACE_Message_Block *mb = new ACE_Message_Block (4096); 
        int n = ACE_OS::read (h, mb->rd_ptr (), mb->capacity ()); 
        //int n = ACE_OS::scanf (mb->rd_ptr ()); 
        //int n = ::sscanf (mb->rd_ptr (), "%s\n"); 
        if (n > 0)
        {
            mb->rd_ptr ()[n] = 0; 
            mb->wr_ptr (n+1); 
            ACE_DEBUG ((LM_DEBUG, "total length %d\n", n+1)); 
            return reply (mb); 
        }

        return 0; 
    }
}; 


class HA_Client : public ACE_Module <ACE_MT_SYNCH>
{
public:
    HA_Client(void);
    ~HA_Client(void);
};
