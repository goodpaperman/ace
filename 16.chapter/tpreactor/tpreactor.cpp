// tpreactor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Event_Handler.h" 
#include "ace/Acceptor.h" 
#include "ace/SOCK_Acceptor.h" 
#include "ace/SOCK_Connector.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/INET_Addr.h" 

#define THR_NUM 10
#define PORT 10020
#define USE_WFMO

#if defined (USE_WFMO)
#include "ace/WFMO_Reactor.h" 
#else 
#include "ace/TP_Reactor.h" 
#endif 

static int reactor_event_hook (ACE_Reactor *)
{
    ACE_DEBUG ((LM_DEBUG, "(%t) handling events\n")); 
    return 0; 
}

class ServerTP : public ACE_Task_Base 
{
public:
    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) Running the event loop\n")); 
        int result = ACE_Reactor::instance ()->run_reactor_event_loop (reactor_event_hook); 
        if (result == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "(%t) %p\n", "Error handling events"), 0); 

        ACE_DEBUG ((LM_DEBUG, "(%t) Done handling events.\n")); 
        return 0; 
    }
}; 

class Client : public ACE_Task_Base 
{
public:
    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) Running the client\n")); 
        ACE_SOCK_Connector connector; 
        ACE_SOCK_Stream stream; 
        ACE_INET_Addr addr(PORT, "localhost"); 
        if (connector.connect (stream, addr) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), -1); 

        ACE_DEBUG ((LM_DEBUG, "connected with %u\n", PORT)); 
        int len = 0, n = 0; 
        char buf[1024] = { 0 }; 
        while (1)
        {
            ACE_OS::sprintf (buf, "this is message %u\n", ++n); 
            len = ACE_OS::strlen (buf) + 1; 
            if (stream.send_n (buf, len) == -1)
                break; 

            ACE_DEBUG ((LM_DEBUG, "(%t) send message %u\n", n)); 
            if (n > 10)
                break; 
        }

        ACE_OS::sleep (1); 
        stream.close (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) exit the client\n")); 
        ACE_Reactor::instance ()->end_reactor_event_loop (); 
        return 0; 
    }
}; 


class Handler : public ACE_Svc_Handler <ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
public:
    virtual int open (void *arg)
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) handler opened.\n")); 
        return ACE_Svc_Handler::open (arg); 
    }

    virtual int handle_input (ACE_HANDLE h)
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) handler %u handle_input.\n", h)); 

        int len = 1024; 
        char buf[1024] = { 0 }; 
        len = peer ().recv (buf, len); 
        if (len <= 0)
            return -1; 

        ACE_DEBUG ((LM_DEBUG, "(%t) recv msg: [%u] %s", len, buf)); 
        return 0; 
    }
}; 

typedef ACE_Acceptor <Handler, ACE_SOCK_ACCEPTOR> ACCEPTOR; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#if defined (USE_WFMO)
    ACE_WFMO_Reactor sr; 
#else 
    ACE_TP_Reactor sr; 
#endif 
    ACE_Reactor reactor (&sr); 
    ACE_Reactor::instance (&reactor); 

    ACCEPTOR acceptor; 
    ACE_INET_Addr addr (PORT, "localhost"); 
    if (acceptor.open (addr) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "open"), -1); 

    ACE_DEBUG ((LM_DEBUG, "(%t) Acceptor opened on %u\n", PORT)); 
    ACE_DEBUG ((LM_DEBUG, "(%t) Spawing %d server threads\n", THR_NUM)); 

    ServerTP pool; 
    pool.activate (THR_NEW_LWP | THR_JOINABLE, THR_NUM); 

    Client client; 
    client.activate (); 

    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

