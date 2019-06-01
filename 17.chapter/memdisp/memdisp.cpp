// memdisp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/MEM_Addr.h" 
#include "ace/MEM_Stream.h" 
#include "ace/MEM_Acceptor.h" 
#include "ace/MEM_Connector.h" 
#include "ace/Acceptor.h" 
#include "ace/Connector.h" 
#include "ace/Select_Reactor.h" 
#include "ace/Reactor.h" 

//#define SINGLE_SIDE 


typedef ACE_Svc_Handler <ACE_MEM_Stream, ACE_MT_SYNCH> HANDLER; 

class MEM_Client : public HANDLER
{
public:
    virtual int open (void *arg = 0)
    {
        index_ = 0; 
        ACE_DEBUG ((LM_DEBUG, "accept a new mem stream\n")); 
        return HANDLER::open (arg); 
    }

    virtual int handle_input (ACE_HANDLE h = ACE_INVALID_HANDLE)
    {
        int ret = 0; 
        char buf[1024] = { 0 }; 

        ret = peer ().recv (buf, sizeof (buf)); 
        if (ret <= 0)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "recv")); 
            return -1; 
        }

        ACE_DEBUG ((LM_DEBUG, "recv: %s", buf)); 

#if !defined (SINGLE_SIDE)
        ACE_OS::sprintf (buf, "reply of %d\n", ++index_); 
        ret = peer ().send (buf, 20); 
        if (ret < 20)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "send")); 
            return -1; 
        }
#endif 
        return 0; 
    }

private:
    int index_; 
}; 


typedef ACE_Acceptor <MEM_Client, ACE_MEM_ACCEPTOR> ACCEPTOR; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_Select_Reactor impl; 
    ACE_Reactor reactor (&impl, 0); 
    ACE_Reactor::instance (&reactor); 

    ACCEPTOR acceptor; 
    acceptor.acceptor ().mmap_prefix ("C:\\memio"); 
    // if we don't add following line, 
    // we will extend file frequently, 
    // which may lead to ACCESS_VIOLATION exceptions.
    acceptor.acceptor ().init_buffer_size (1024 * 1024); 
    // Won't work under Select_Reactor.
    //acceptor.acceptor ().preferred_strategy (ACE_MEM_IO::MT); 

    ACE_MEM_Addr addr (10021); 
    if (acceptor.open (addr, &reactor) == -1)
        ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "acceptor open"), -1); 

    ACE_DEBUG ((LM_DEBUG, "acceptor opened\n")); 
    reactor.run_reactor_event_loop (); 
    acceptor.close (); 
	return 0;
}


