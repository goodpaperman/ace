// recursive.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//#define USE_REMOTE

#if defined(USE_REMOTE)
#  include "ace/Remote_Tokens.h" 
#else 
#  include "ace/Local_Tokens.h" 
#endif 

#include "ace/Thread_Manager.h" 
#include "ace/Guard_T.h" 
#include "ace/OS_NS_unistd.h" 

ACE_THR_FUNC_RETURN thr_func(void* arg)
{
    ACE_DEBUG((LM_DEBUG, "(%t) thr_func running\n")); 
#if defined(USE_REMOTE)
    ACE_Remote_Mutex* mutex = (ACE_Remote_Mutex*)arg; 
#else 
    ACE_Local_Mutex* mutex = (ACE_Local_Mutex*)arg; 
#endif 
    //int result = mutex->tryacquire(); 
    int result = mutex->acquire(); 
    if(result == 0)
    {
        ACE_DEBUG((LM_DEBUG, "(%t) acquire lock!\n")); 
        mutex->release(); 
        ACE_DEBUG((LM_DEBUG, "(%t) release lock!\n")); 
    }
    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#if defined(USE_REMOTE)
    ACE_Remote_Mutex::set_server_address(ACE_INET_Addr(10202, ACE_LOCALHOST)); 
    ACE_Remote_Mutex mutex(0, 0, 1); 
#else 
    ACE_Local_Mutex mutex(0, 0, 1); 
#endif 

    ACE_DEBUG((LM_DEBUG, "client id: %s\n", mutex.client_id())); 

    {
#if defined(USE_REMOTE)
        ACE_GUARD_RETURN(ACE_Remote_Mutex, mon1, mutex, -1); 
#else 
        ACE_GUARD_RETURN(ACE_Local_Mutex, mon1, mutex, -1); 
#endif 

        ACE_DEBUG((LM_DEBUG, "in first guard: %s\n", mutex.owner_id())); 

        {
#if defined(USE_REMOTE)
            ACE_GUARD_RETURN(ACE_Remote_Mutex, mon2, mutex, -1); 
#else 
            ACE_GUARD_RETURN(ACE_Local_Mutex, mon2, mutex, -1); 
#endif 

            ACE_DEBUG((LM_DEBUG, "in second guard: %s\n", mutex.owner_id())); 

            {
#if defined(USE_REMOTE)
                ACE_GUARD_RETURN(ACE_Remote_Mutex, mon3, mutex, -1); 
#else 
                ACE_GUARD_RETURN(ACE_Local_Mutex, mon3, mutex, -1); 
#endif 

                ACE_DEBUG((LM_DEBUG, "in third guard: %s\n", mutex.owner_id())); 
            }
        }

        ACE_Thread_Manager::instance()->spawn(thr_func, &mutex, THR_JOINABLE | THR_NEW_LWP/* | THR_SUSPENDED*/); 
        //ACE_OS::sleep(1); 
        //mutex.renew(); 
        ACE_OS::sleep(10); 
    }
    
    ACE_Thread_Manager::instance()->wait(); 
	return 0;
}

