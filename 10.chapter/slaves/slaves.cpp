// slaves.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Process_Manager.h" 
#include "ace/os_ns_unistd.h" 

static const int NCHILDREN = 2; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if(argc > 1)
    {
        ACE_OS::sleep(100); 
        return 77881314; 
    }
    else 
    {
        ACE_Process_Manager* pm = ACE_Process_Manager::instance(); 

        ACE_Process_Options options; 
        options.command_line(ACE_TEXT("%s a"), argv[0]); 

        pid_t pids[NCHILDREN] = { 0 }; 
        if(pm->spawn_n(NCHILDREN, options, pids) == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p"), ACE_TEXT("spawn_n")), -1); 

        pm->terminate(pids[0]); 

        ACE_exitcode status; 
        pm->wait(pids[0], &status); 

#if !defined(ACE_WIN32)
        if(WIFSIGNALED(status) != 0)
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%d died because of a signal of type %d\n"), pids[0], WTERMSIG(status))); 
#else
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("The process terminated with exit code %d\n"), status)); 
#endif 

        //pm->wait(0); 
        pm->wait(0, &status); 

#if !defined(ACE_WIN32)
        if(WIFSIGNALED(status) != 0)
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("child process died because of a signal of type %d\n"), WTERMSIG(status))); 
#else
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("The process terminated with exit code %d\n"), status)); 
#endif 
    }

	return 0;
}

