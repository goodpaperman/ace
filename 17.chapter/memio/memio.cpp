// memio.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/MEM_Addr.h" 
#include "ace/MEM_Stream.h" 
#include "ace/MEM_Acceptor.h" 
#include "ace/MEM_Connector.h" 
#include "ace/Reactor.h" 

//#define USE_MT

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_MEM_Addr addr (10021); 
    ACE_MEM_Acceptor acceptor; 
    acceptor.mmap_prefix ("C:\\memio"); 
    acceptor.init_buffer_size (1024*1024); // 1MB
#if defined (USE_MT)
    acceptor.preferred_strategy (ACE_MEM_IO::MT); 
#endif 
    ACE_MEM_Stream stream; 
    if (acceptor.open (addr) == -1)
        ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "acceptor open"), -1); 

    ACE_DEBUG ((LM_DEBUG, "acceptor opened\n")); 
    if (acceptor.accept (stream) == -1)
        ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "accept"), -1); 

    ACE_DEBUG ((LM_DEBUG, "accept a new mem stream\n")); 

    int ret = 0, n = 0; 
    char buf[1024] = { 0 }; 
    while (1)
    {
        ret = stream.recv (buf, sizeof (buf)); 
        if (ret <= 0)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "recv")); 
            break; 
        }

        ACE_DEBUG ((LM_DEBUG, "recv: %s", buf)); 
        ACE_OS::sprintf (buf, "reply of %d\n", ++n); 
        ret = stream.send (buf, ACE_OS::strlen (buf) + 1); 
        if (ret < ACE_OS::strlen (buf) + 1)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "send")); 
            break; 
        }
    }

    stream.close (); 
    acceptor.close (); 
	return 0;
}

