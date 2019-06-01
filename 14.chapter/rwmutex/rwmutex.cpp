// rwmutex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/OS_NS_unistd.h" 
#include "ace/Task.h" 

#define RD_CNT 2
#define WR_CNT 1
#define RDWR_CNT 1
//#define USE_THR

#if defined (USE_THR)
#include "ace/RW_Thread_Mutex.h" 
typedef ACE_RW_Thread_Mutex MUTEX_TYPE; 
#else 
#include "ace/RW_Mutex.h" 
typedef ACE_RW_Mutex MUTEX_TYPE; 
#endif 

int g_counter = 0; 

class rd_task : public ACE_Task_Base
{
public:
    rd_task(MUTEX_TYPE& rdlk)
        : rdlk_(rdlk)
    {
    }

    virtual int svc() 
    {
        ACE_DEBUG((LM_DEBUG, "(%T %t) rd_task running.\n")); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            ACE_OS::sleep (ACE_Time_Value (0, 100000)); 
            //ACE_READ_GUARD_RETURN(MUTEX_TYPE, mon, rdlk_, -1); 
            rdlk_.acquire_read (); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) acquire the read lock, counter = %u.\n", g_counter)); 
            rdlk_.release (); 
        }

        return 0; 
    }

private:
    MUTEX_TYPE& rdlk_; 
}; 

class wr_task : public ACE_Task_Base
{
public:
    wr_task(MUTEX_TYPE& wrlk)
        : wrlk_(wrlk)
    {
    }

    virtual int svc()
    {
        ACE_DEBUG((LM_DEBUG, "(%T %t) wr_task running.\n")); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            ACE_OS::sleep (ACE_Time_Value (0, 200000)); 
            //ACE_WRITE_GUARD_RETURN(MUTEX_TYPE, mon, wrlk_, -1); 
            wrlk_.acquire_write (); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) acquire the write lock, set counter = %u.\n", ++g_counter)); 
            wrlk_.release (); 
        }

        return 0; 
    }

private:
    MUTEX_TYPE& wrlk_; 
}; 

class rdwr_task : public ACE_Task_Base
{
public:
    rdwr_task(MUTEX_TYPE& lk)
        : lk_(lk)
    {
    }

    virtual int svc()
    {
        ACE_DEBUG((LM_DEBUG, "(%T %t) rdwr_task running.\n")); 
        while(thr_mgr()->testcancel(ACE_Thread::self()) == 0)
        {
            ACE_OS::sleep (ACE_Time_Value (0, 400000)); 
            //ACE_WRITE_GUARD_RETURN(MUTEX_TYPE, mon, lk_, -1); 
            lk_.acquire_read (); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) acquire the read lock, counter = %u.\n", g_counter)); 
            ACE_OS::sleep (ACE_Time_Value (0, 400000)); 
            lk_.tryacquire_write_upgrade (); 
            ACE_DEBUG((LM_DEBUG, "(%T %t) update the read lock to write, set counter = %u.\n", ++g_counter)); 
            lk_.release (); 
        }

        return 0; 
    }

private:
    MUTEX_TYPE& lk_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    MUTEX_TYPE mutex; 

    rd_task rdt(mutex); 
    wr_task wrt(mutex); 
    rdwr_task rdwrt (mutex); 

    rdt.activate(THR_JOINABLE | THR_NEW_LWP, RD_CNT);
    wrt.activate(THR_JOINABLE | THR_NEW_LWP, WR_CNT); 
    rdwrt.activate(THR_JOINABLE | THR_NEW_LWP, RDWR_CNT); 

    ACE_OS::sleep(3); 
    ACE_Thread_Manager::instance()->cancel_all(); 

    wrt.wait(); 
    rdt.wait(); 
    rdwrt.wait (); 
    return 0;
}

