// condition.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/OS_NS_unistd.h" 

#define USE_RECUR
#define NUM_THRS 2
//#define USE_SIGNAL


#if defined (USE_RECUR)
#include "ace/Condition_Recursive_Thread_Mutex.h" 
typedef ACE_Recursive_Thread_Mutex MUTEX_TYPE; 
typedef ACE_Condition_Recursive_Thread_Mutex COND_TYPE; 
#else 
#include "ace/Condition_Thread_Mutex.h" 
typedef ACE_Thread_Mutex MUTEX_TYPE; 
typedef ACE_Condition_Thread_Mutex COND_TYPE; 
#endif 

class worker : public ACE_Task_Base
{
public:
    worker (COND_TYPE &cond)
        : cond_ (cond)
    {
    }

    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) worker started.\n")); 
        cond_.mutex ().acquire (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) prepare wait.\n")); 
        cond_.wait (); 
        cond_.mutex ().release (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) worker ended.\n")); 
        return 0; 
    }

private:
    COND_TYPE &cond_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    MUTEX_TYPE mutex; 
    COND_TYPE cond (mutex); 

    worker w (cond); 
    w.activate (THR_NEW_LWP | THR_JOINABLE, NUM_THRS); 

    ACE_OS::sleep (1); 
    mutex.acquire (); 
#if defined (USE_SIGNAL)
    cond.signal (); 
    ACE_DEBUG ((LM_DEBUG, "(%t) condition signaled.\n")); 
#else 
    cond.broadcast (); 
    ACE_DEBUG ((LM_DEBUG, "(%t) condition broadcasted.\n")); 
#endif 
    mutex.release (); 

    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

