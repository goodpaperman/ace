// deadlock.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Local_Tokens.h" 
#include "ace/Token_Manager.h" 

//#define IGNORE_DEADLOCK

class task : public ACE_Task_Base
{
public:
    task(char const* name1, char const* name2)
        : name1_(name1)
        , name2_(name2)
    {
    }

    virtual int svc()
    {
#if defined(IGNORE_DEADLOCK)
        ACE_Local_Mutex mutex1(name1_, 1, 1); 
        ACE_Local_Mutex mutex2(name2_, 1, 1); 
#else 
        ACE_Local_Mutex mutex1(name1_, 0, 1); 
        ACE_Local_Mutex mutex2(name2_, 0, 1); 
#endif 

        int result = mutex1.acquire(); 
        ACE_DEBUG((LM_DEBUG, "(%t) mutex1 acquired %d!\n", result)); 
        ACE_OS::sleep(2); 

        {
            int result = mutex2.acquire(); 
            ACE_DEBUG((LM_DEBUG, "(%t) mutex2 acquired %d, %u!\n", result, ACE_OS::last_error())); 
            if(result == 0)
                mutex2.release(); 
        }

        if(result == 0)
            mutex1.release(); 

        return 0; 
    }

private:
    char const* name1_; 
    char const* name2_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    // must call this to see the dead lock dump.
    ACE_Token_Manager::instance()->debug(1); 

    task t1("A", "B"); 
    task t2("B", "A"); 

    t1.activate(); 
    ACE_OS::sleep(1); 
    t2.activate(); 

    t1.wait(); 
    t2.wait(); 
	return 0;
}

