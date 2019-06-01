// rwlock.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/OS_NS_unistd.h" 
#include "ace/Local_Tokens.h" 
#include "ace/Task.h" 

int g_counter = 0; 

class rd_task : public ACE_Task_Base
{
public:
    rd_task(ACE_Local_RLock& rdlk)
        : rdlk_(rdlk)
    {
    }

    virtual int svc() 
    {
        ACE_DEBUG((LM_DEBUG, "(%T %t) rd_task running.\n")); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            ACE_READ_GUARD_RETURN(ACE_Local_RLock, mon, rdlk_, -1); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) acquire the read lock, counter = %u.\n", g_counter)); 
        }

        return 0; 
    }

private:
    ACE_Local_RLock& rdlk_; 
}; 

class wr_task : public ACE_Task_Base
{
public:
    wr_task(ACE_Local_WLock& wrlk)
        : wrlk_(wrlk)
    {
    }

    virtual int svc()
    {
        ACE_DEBUG((LM_DEBUG, "(%T %t) wr_task running.\n")); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            ACE_WRITE_GUARD_RETURN(ACE_Local_WLock, mon, wrlk_, -1); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) acquire the write lock, set counter = %u.\n", ++g_counter)); 
        }

        return 0; 
    }

private:
    ACE_Local_WLock& wrlk_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_Local_RLock rd_lock("the_same_token", 0, 1); 
    ACE_Local_WLock wr_lock("the_same_token", 0, 1); 

    rd_task rdt(rd_lock); 
    wr_task wrt(wr_lock); 

    rdt.activate(THR_JOINABLE | THR_NEW_LWP, 2);
    wrt.activate(THR_JOINABLE | THR_NEW_LWP, 2); 

    ACE_OS::sleep(30); 
    ACE_Thread_Manager::instance()->cancel_all(); 

    wrt.wait(); 
    rdt.wait(); 
    return 0;
}

