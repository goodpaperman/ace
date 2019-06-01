// tss_scene1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_unistd.h" 

#define USE_THR_MGR
//#define SCENE_MAIN_TERM
//#define SCENE_MAIN_EXIT
//#define SCENE_THR_EXIT

#if defined (USE_THR_MGR)
#include "ace/Thread_Manager.h" 
#else 
#include "ace/OS_NS_thread.h" 
#endif 

ACE_thread_key_t key = 0; 

void tss_cleanup_func (void *data)
{
    ACE_DEBUG ((LM_DEBUG, "(%t) tss data (%u) cleaned up.\n", *(int *)data)); 
    delete (int *)data; 
}

ACE_THR_FUNC_RETURN thr_func (void *arg)
{
    ACE_DEBUG ((LM_DEBUG, "(%t) start\n")); 
    ACE_OS::thr_setspecific (key, new int (ACE_OS::thr_self ())); 
#if defined (SCENE_MAIN_TERM)
    ACE_OS::sleep (10); 
#elif defined (SCENE_THR_EXIT)
    ACE_DEBUG ((LM_DEBUG, "(%t) exit\n")); 
    ACE_OS::thr_exit (2); 
#endif 
    ACE_DEBUG ((LM_DEBUG, "(%t) end\n")); 
    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_DEBUG ((LM_DEBUG, "(%t) main start\n")); 
    ACE_OS::thr_keycreate (&key, tss_cleanup_func); 

#if defined (USE_THR_MGR)
    ACE_Thread_Manager::instance ()->spawn (thr_func); 
#else 
    ACE_hthread_t hid = 0; 
    ACE_OS::thr_create (thr_func, 0, THR_NEW_LWP | THR_JOINABLE, 0, &hid); 
#endif 

#if defined (SCENE_MAIN_TERM)
    ACE_OS::sleep (3); 
#elif defined (SCENE_MAIN_EXIT)
    ACE_OS::exit (1); 
#else
#  if defined (USE_THR_MGR)
    ACE_Thread_Manager::instance ()->wait (); 
#  else 
    ACE_OS::thr_join (hid, 0); 
#  endif 
#endif 

    ACE_DEBUG ((LM_DEBUG, "(%t) main end\n")); 
	return 0;
}

