// synchronizer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Process.h" 
#include "ace/Process_Manager.h" 
#include "ace/Process_Mutex.h" 
#include "ace/Process_Semaphore.h" 
#include "ace/Log_Msg.h" 
#include "ace/os_ns_unistd.h" 

#define USE_PROCESS_MANAGER
//#define USE_PROCESS_SEMAPHORE

#if defined(USE_PROCESS_SEMAPHORE)
#define ACE_SYNCH_OBJ ACE_Process_Semaphore
#else 
#define ACE_SYNCH_OBJ ACE_Process_Mutex
#endif 

class GResourceUser
{
public:
    GResourceUser(ACE_SYNCH_OBJ& synchronizer) : gsynchronizer_(synchronizer) 
    {
        ACE_TRACE(ACE_TEXT("GResourceUser::GResourceUser")); 
    }

    int run()
    {
        ACE_TRACE(ACE_TEXT("GResourceUser::run")); 

        int count = 0; 
        while(count++ < 10)
        {
            ACE_GUARD_RETURN(ACE_SYNCH_OBJ, guard, gsynchronizer_, -1); 
            //int result = this->gsynchronizer_.acquire(); 
            //ACE_ASSERT(result == 0); 

            ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) has the synchronizer\n"))); 
            ACE_OS::sleep(1); 

            //result = this->gsynchronizer_.release(); 
            //ACE_ASSERT(result == 0); 
            ////ACE_OS::sleep(1); 
        }

        return 0; 
    }

private:
    ACE_SYNCH_OBJ& gsynchronizer_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if(argc > 1)
    {
#if defined(USE_PROCESS_SEMAPHORE)
        ACE_SYNCH_OBJ synchronizer(1, "GlobalSemaphore"); 
#else 
        ACE_SYNCH_OBJ synchronizer("GlobalMutex"); 
#endif 
        GResourceUser acquirer(synchronizer); 
        acquirer.run(); 

        // for semaphores, we must remove it explicitly, 
        // for mutexes, they will get removed when destructed.
        synchronizer.remove(); 
    }
    else 
    {
        ACE_Process_Options options; 
        options.command_line(ACE_TEXT("%s a"), argv[0]); 

#if !defined(USE_PROCESS_MANAGER)
        ACE_Process processa, processb; 
        pid_t pida = processa.spawn(options); 
        pid_t pidb = processb.spawn(options); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Spawned processes; pids %d: %d\n"), pida, pidb)); 

        if(processa.wait() == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("processa wait")), -1); 

        if(processb.wait() == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("processb wait")), -1); 
#else 
        int const PROC_NUM = 5; 
        pid_t pids[PROC_NUM] = { 0 }; 
        ACE_Process_Manager man; 
        if(man.spawn_n(PROC_NUM, options, pids) == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("spawn_n")), -1); 

        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Spawned processes: \n"))); 
        for(int i=0; i<PROC_NUM; ++ i)
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d "), pids[i])); 

        ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
        if(man.wait() == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("wait")), -1); 
#endif 
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) exiting.\n"))); 
    return 0;
}

