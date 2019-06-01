// pool2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Condition_Thread_Mutex.h" 
#include "ace/OS_NS_time.h" 
#include "ace/OS_NS_unistd.h" 


class Workers : public ACE_Task <ACE_MT_SYNCH>
{
public:
    enum { 
        WORK_TIME = 100, 
    }; 

    Workers ()
    {
    }

    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) worker started.\n")); 

        while (1)
        {
            ACE_Message_Block *mb = 0; 
            if(getq (mb) == -1)
            {
                ACE_DEBUG ((LM_DEBUG, "(%t) shutting down\n")); 
                break; 
            }

            process_message (mb); 
        }

        return 0; 
    }

    void process_message (ACE_Message_Block *mb)
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) process message: %s.\n", mb->base())); 
        mb->release (); 
        ACE_OS::sleep (ACE_Time_Value (0, WORK_TIME)); 
    }
}; 



class Manager : public ACE_Task <ACE_MT_SYNCH>
{
public:
    enum 
    {
        POOL_SIZE = 5, 
        MAX_TIMEOUT = 100, 
    }; 

    Manager ()
        : shutdown_ (0)
    {
    }

    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) Manager started\n")); 

        Workers pool; 
        pool.activate (THR_NEW_LWP | THR_JOINABLE, POOL_SIZE); 

        while (!done ())
        {
            ACE_Time_Value tv (0, (long) MAX_TIMEOUT); 
            tv += ACE_OS::time (0); 

            ACE_Message_Block *mb = 0; 
            if (getq (mb, &tv) == -1)
            {
                shutdown (); 
                pool.msg_queue ()->deactivate (); 
                pool.wait ();
                break; 
            }

            pool.putq (mb); 
        }

        return 0; 
    }

    int shutdown ()
    {
        ACE_DEBUG ((LM_DEBUG, "prepare to shutdown\n")); 
        shutdown_ = 1; 
        return 0; 
    }

    int done ()
    {
        return shutdown_; 
    }

private:
    int shutdown_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    Manager mgr; 
    mgr.activate (); 

    ACE_Message_Block *mb = 0; 
    for (int i=0; i<10000; ++ i)
    {
        ACE_NEW_RETURN (mb, ACE_Message_Block (16), -1); 
        mb->wr_ptr (ACE_OS::sprintf (mb->base (), "%d", i+1)); 
        mgr.putq (mb); 
    }

    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

