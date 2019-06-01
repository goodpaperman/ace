// recurmutex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#define USE_RECUR

#if defined(USE_RECUR)
#  include "ace/Recursive_Thread_Mutex.h" 
#else 
#  include "ace/Thread_Mutex.h" 
#endif 

#include "ace/Thread_Manager.h" 
#include "ace/Guard_T.h" 
#include "ace/OS_NS_unistd.h" 

ACE_THR_FUNC_RETURN thr_func(void* arg)
{
    ACE_DEBUG((LM_DEBUG, "(%t) thr_func running\n")); 
#if defined(USE_RECUR)
    ACE_Recursive_Thread_Mutex* mutex = (ACE_Recursive_Thread_Mutex*)arg; 
#else 
    ACE_Thread_Mutex* mutex = (ACE_Thread_Mutex*)arg; 
#endif 
    //int result = mutex->tryacquire(); 
    int result = mutex->acquire(); 
    if(result == 0)
    {
        ACE_DEBUG((LM_DEBUG, "(%t) acquire lock!\n")); 
#if defined (USE_RECUR)
        ACE_DEBUG((LM_DEBUG, 
            "(%t) on acquire: %d, %d\n", 
            mutex->get_thread_id (), 
            mutex->get_nesting_level ())); 
#endif 
        mutex->release(); 
        ACE_DEBUG((LM_DEBUG, "(%t) release lock!\n")); 
    }
    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#if defined(USE_RECUR)
    ACE_Recursive_Thread_Mutex mutex; 
    ACE_DEBUG((LM_DEBUG, "(%t) recursive count: %d\n", mutex.get_nesting_level())); 
#else 
    ACE_Thread_Mutex mutex; 
#endif 

    {
#if defined(USE_RECUR)
        ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, mon1, mutex, -1); 
        ACE_DEBUG((LM_DEBUG, 
            "(%t) in first guard: %d, %d\n", 
            mutex.get_thread_id (), 
            mutex.get_nesting_level())); 
#else 
        ACE_GUARD_RETURN(ACE_Thread_Mutex, mon1, mutex, -1); 
#endif 

        {
#if defined(USE_RECUR)
            ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, mon2, mutex, -1); 
            ACE_DEBUG((LM_DEBUG, 
                "(%t) in second guard: %d, %d\n", 
                mutex.get_thread_id (), 
                mutex.get_nesting_level ())); 
#else 
            ACE_GUARD_RETURN(ACE_Thread_Mutex, mon2, mutex, -1); 
#endif 

            {
#if defined(USE_RECUR)
                ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, mon3, mutex, -1); 
                ACE_DEBUG((LM_DEBUG, 
                    "(%t) in third guard: %d, %d\n", 
                    mutex.get_thread_id (), 
                    mutex.get_nesting_level ())); 
#else 
                ACE_GUARD_RETURN(ACE_Thread_Mutex, mon3, mutex, -1); 
#endif 
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

