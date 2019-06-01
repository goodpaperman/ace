// unmanaged.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Service_Config.h" 
#include "ace/Signal.h" 

void __cdecl exit_handler (int)
{
    ACE_DEBUG ((LM_DEBUG, "exiting...\n")); 
    ACE_Reactor::instance ()->end_reactor_event_loop (); 
}

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    int ret = ACE_Service_Config::open (argc, argv, ACE_DEFAULT_LOGGER_KEY, 0); 
    if (ret != 0)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "Service_Config open"), -1); 

    ACE_Sig_Action sigact (exit_handler, SIGINT); 
    ACE_Reactor::instance ()->run_reactor_event_loop (&ACE_Reactor::check_reconfiguration); 
	return 0;
}

