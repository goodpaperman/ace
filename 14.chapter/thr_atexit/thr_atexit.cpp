// thr_atexit.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Thread_Manager.h" 

//#define USE_EXIT

void tss_cleanup (void *obj, void *param)
{
    ACE_DEBUG ((LM_DEBUG, "(%t) clean up 0x%x (%u).\n", obj, *(int *)param)); 
    delete (int *)param; 
}

ACE_THR_FUNC_RETURN thr_func (void *arg)
{
    ACE_DEBUG ((LM_DEBUG, "(%t) start\n")); 
    ACE_Thread_Manager *mgr = ACE_Thread_Manager::instance (); 
    mgr->at_exit (0, tss_cleanup, arg); 

#if defined (USE_EXIT)
    ACE_DEBUG ((LM_DEBUG, "(%t) exit\n")); 
    ACE_OS::thr_exit (1); 
#endif 
    ACE_DEBUG ((LM_DEBUG, "(%t) end\n")); 
    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_DEBUG ((LM_DEBUG, "(%t) main start\n")); 
    ACE_Thread_Manager *mgr = ACE_Thread_Manager::instance (); 
    mgr->spawn (thr_func, new int(42)); 
    mgr->wait (); 
    ACE_DEBUG ((LM_DEBUG, "(%t) main end\n")); 
	return 0;
}

