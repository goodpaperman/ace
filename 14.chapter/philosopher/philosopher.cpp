// philosopher.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//#define USE_REMOTE

#if defined(USE_REMOTE)
#  include "ace/Remote_Tokens.h" 
#else 
#  include "ace/Local_Tokens.h" 
#endif 

#include "ace/Token_Manager.h" 
#include "ace/Thread_Manager.h" 
#include "ace/Atomic_Op.h" 
#include "ace/Task.h" 

#define MAX_NUM 5
ACE_Atomic_Op<ACE_Thread_Mutex, long> current_sit = 0; 

#if defined(USE_REMOTE)
ACE_Remote_Mutex* 
#else 
ACE_Local_Mutex* 
#endif 
chopsticks[MAX_NUM] = { 0 }; 

class philosopher : public ACE_Task_Base
{
public:
    virtual int svc()
    {
        int result = 0; 
        int sit = current_sit ++; 
        int left = sit; 
        int right = (sit + 1) % MAX_NUM; 
        ACE_DEBUG((LM_DEBUG, "(%t) takes sit %u, left %u, right %u.\n", sit, left, right)); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            result = chopsticks[left]->acquire(); 
            if(result == 0)
            {
                //ACE_OS::sleep(3); 
                ACE_DEBUG((LM_DEBUG, "(%t) takes left chopstick.\n")); 
                result = chopsticks[right]->acquire(); 
                if(result == 0)
                {
                    ACE_DEBUG((LM_DEBUG, "(%t) takes right chopstick, start eating.\n")); 
                    //ACE_OS::sleep(1); 
                    chopsticks[right]->release(); 
                }

                chopsticks[left]->release(); 
                ACE_DEBUG((LM_DEBUG, "(%t) release both chopsticks.\n")); 
            }
        }

        return 0; 
    }
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    philosopher p; 
    char name[4] = { 0 }; 
    ACE_Token_Manager::instance()->debug(1); 

#if defined(USE_REMOTE)
    ACE_Remote_Mutex::set_server_address(ACE_INET_Addr(10202, ACE_LOCALHOST)); 
#endif 

    for(int i=0; i<MAX_NUM; ++ i)
    {
        ACE_OS::sprintf(name, "%c", 'A'+i); 
        chopsticks[i] = new 
#if defined(USE_REMOTE)
            ACE_Remote_Mutex(name, 0, 0); 
#else 
            ACE_Local_Mutex(name, 0, 0); 
#endif 
    }

    p.activate(THR_NEW_LWP | THR_JOINABLE, MAX_NUM); 
    ACE_OS::sleep(10); 

    ACE_Thread_Manager::instance()->cancel_all(); 
    p.wait(); 

    for(int i=0; i<MAX_NUM; ++ i)
    {
        delete chopsticks[i]; 
    }

	return 0;
}

