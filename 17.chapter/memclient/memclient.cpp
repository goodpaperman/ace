// memclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/MEM_Addr.h" 
#include "ace/MEM_Stream.h" 
#include "ace/MEM_Acceptor.h" 
#include "ace/MEM_Connector.h" 
#include "ace/Reactor.h" 

#define USE_MT
//#define SINGLE_SIDE
#define MALLOC_DUMP

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_INET_Addr addr ((unsigned short)10021, "192.168.19.107"); 
    ACE_MEM_Connector connector; 
#if defined (USE_MT)
    connector.preferred_strategy (ACE_MEM_IO::MT); 
#endif 
    ACE_MEM_Stream stream; 
    if (connector.connect (stream, addr) == -1)
        ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "connect"), -1); 

    ACE_DEBUG ((LM_DEBUG, "connect a new mem stream\n")); 

    int len = 0, n = 0; 
    char buf[128] = { 0 }; 
    while (1)
    {
        ACE_OS::sprintf (buf, "this is %d\n", ++n); 
        len = stream.send (buf, 20); 
        if (len < 0 || len < 20)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "send")); 
            break; 
        }

#if defined (SINGLE_SIDE)
        ACE_DEBUG ((LM_DEBUG, "send: %s", buf)); 
#else 
        len = stream.recv (buf, sizeof (buf)-1); 
        if (len <= 0)
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "recv")); 
            break; 
        }

        ACE_DEBUG ((LM_DEBUG, "recv: %s", buf)); 
#endif 

#if defined (MALLOC_DUMP)
        stream.allocator ()->print_stats (); 
#else 
        //ACE_OS::sleep (ACE_Time_Value (0, 10000)); 
#endif 
    }

    stream.close (); 
    return 0;
}

