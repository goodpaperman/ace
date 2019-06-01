// barrier.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Barrier.h" 
#include "ace/Task_T.h" 
#include "ace/OS_NS_unistd.h" 

#define HAS_BARRIER

class HA_CommandHandler : public ACE_Task <ACE_MT_SYNCH>
{
public: 
    enum { 
        N_THREADS = 5
    }; 

    HA_CommandHandler (ACE_Barrier &startup_barrier, 
        ACE_Barrier &shutdown_barrier)
        : startup_barrier_ (startup_barrier)
        , shutdown_barrier_ (shutdown_barrier)
    {
    }

    void init_handler ()
    {
        ACE_OS::srand (ACE_Thread::self ()); 
        ACE_OS::sleep (ACE_Time_Value (0, ACE_OS::rand () % 1000000)); 
    }

    int handle_requests ()
    {
        for (int i=0; i<128; ++ i)
        {
            ACE_OS::sleep (ACE_Time_Value (0, ACE_OS::rand () % 100000)); 
            ACE_DEBUG ((LM_DEBUG, "(%t: %T) working %u\n", i)); 
        }

        return -1; 
    }

    virtual int svc ()
    {
        init_handler (); 
#if defined (HAS_BARRIER)
        startup_barrier_.wait (); 
#endif 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t: %D) started\n"))); 

        while(handle_requests () > 0)
            ; 

#if defined (HAS_BARRIER)
        shutdown_barrier_.wait (); 
#endif 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t: %D) ended\n"))); 
        return 0; 
    }

private: 
    ACE_Barrier &startup_barrier_; 
    ACE_Barrier &shutdown_barrier_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_Barrier startup_barrier (HA_CommandHandler::N_THREADS); 
    ACE_Barrier shutdown_barrier (HA_CommandHandler::N_THREADS); 

    HA_CommandHandler handler (startup_barrier, shutdown_barrier); 
    handler.activate (THR_NEW_LWP | THR_JOINABLE, HA_CommandHandler::N_THREADS); 
    handler.wait (); 
	return 0;
}

