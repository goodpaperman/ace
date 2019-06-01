// filelock.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/RW_Process_Mutex.h" 
#include "ace/OS_NS_unistd.h" 
#include "ace/Log_Msg.h" 

//#define READER

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_RW_Process_Mutex mutex ("C:/lock.txt"); 
#if defined (READER)
    int ret = mutex.acquire_read (); 
#else 
    int ret = mutex.acquire_write (); 
#endif 
    if (ret == 0)
    {
#if defined (READER)
        ACE_DEBUG ((LM_DEBUG, "(%P/%t %T) reader acquire the mutex.\n")); 
#else 
        ACE_DEBUG ((LM_DEBUG, "(%P/%t %T) writer acquire the mutex.\n")); 
#endif 
        ACE_OS::sleep (20); 
        mutex.release (); 
        ACE_DEBUG ((LM_DEBUG, "(%P/%t %T) release the mutex.\n")); 
        //ACE_OS::sleep (3); 
    }

	return 0;
}

