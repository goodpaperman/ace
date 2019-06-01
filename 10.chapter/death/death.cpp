// death.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/os_ns_unistd.h" 
#include "ace/Process_Manager.h" 
#include "ace/Event_Handler.h" 
#include "ace/Reactor.h" 

#define NCHILDREN 10
//#define USE_DEFAULT_HANDLER
#define SPAWN_WITH_HANDLER

class DeathHandler : public ACE_Event_Handler
{
public:
    DeathHandler() : count_ (0)
    {
        ACE_TRACE(ACE_TEXT("DeathHandler::DeathHandler")); 
    }

    virtual int handle_exit(ACE_Process* process)
    {
        ACE_TRACE(ACE_TEXT("DeathHandler::handle_exit")); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("Process %d exited with exit code %d\n"), 
            process->getpid(), process->return_value())); 

        if(++count_ == NCHILDREN)
            ACE_Reactor::instance()->end_reactor_event_loop(); 

        return 0; 
    }

    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
    {
        ACE_TRACE(ACE_TEXT("DeathHandler::handle_close")); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("handle %d closed\n"), handle)); 

        return 0; 
    }

private:
    int count_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if(argc > 1)
    {
        ACE_OS::sleep(1); 
        return ACE_OS::getpid(); 
    }

    DeathHandler handler; 
    ACE_Process_Options options; 
    options.command_line(ACE_TEXT("%s a"), argv[0]); 
    ACE_Process_Manager pm(NCHILDREN, ACE_Reactor::instance()); 

#if defined(USE_DEFAULT_HANDLER)
    pm.register_handler(&handler, ACE_INVALID_PID); 
#endif 

    pid_t pids[NCHILDREN] = { 0 }; 
#if defined(SPAWN_WITH_HANDLER) && !defined(USE_DEFAULT_HANDLER)
    if(pm.spawn_n(NCHILDREN, options, pids, &handler) == -1)
#else 
    if(pm.spawn_n(NCHILDREN, options, pids) == -1)
#endif 
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("spawn_n")), -1); 

    //ACE_OS::sleep(1); 
#if !defined(SPAWN_WITH_HANDLER) && !defined(USE_DEFAULT_HANDLER)
    for(int i=0; i<NCHILDREN; ++ i)
        pm.register_handler(&handler, pids[i]); 
#endif 

    ACE_Reactor::instance()->run_reactor_event_loop(); 

	return 0;
}

