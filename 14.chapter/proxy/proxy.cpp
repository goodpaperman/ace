// proxy.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//#define USE_REMOTE

#if defined(USE_REMOTE)
#  include "ace/Remote_Tokens.h" 
#else 
#  include "ace/Local_Tokens.h" 
#endif 

#include "ace/Guard_T.h" 
#include "ace/OS_NS_unistd.h" 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#if defined(USE_REMOTE)
    ACE_Remote_Mutex::set_server_address(ACE_INET_Addr(10202, ACE_LOCALHOST)); 
    ACE_Remote_Mutex mutex1("proxy", 0, 1); 
    ACE_Remote_Mutex mutex2("proxy", 0, 1); 
#else 
    ACE_Local_Mutex mutex1("proxy", 0, 1); 
    ACE_Local_Mutex mutex2("proxy", 0, 1); 
#endif 

    {
#if defined(USE_REMOTE)
        ACE_GUARD_RETURN(ACE_Remote_Mutex, mon1, mutex1, -1); 
#else 
        ACE_GUARD_RETURN(ACE_Local_Mutex, mon1, mutex1, -1); 
#endif 

        ACE_DEBUG((LM_DEBUG, "guard mutex1\n")); 

        {
#if defined(USE_REMOTE)
            ACE_GUARD_RETURN(ACE_Remote_Mutex, mon2, mutex2, -1); 
#else 
            ACE_GUARD_RETURN(ACE_Local_Mutex, mon2, mutex2, -1); 
#endif 

            ACE_DEBUG((LM_DEBUG, "guard mutex2\n")); 
            ACE_OS::sleep(1); 
        }
    }

	return 0;
}

