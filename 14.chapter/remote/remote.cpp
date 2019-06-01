// remote.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Remote_Tokens.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if(argc < 2)
    {
        ACE_DEBUG((LM_DEBUG, "need token names!\n")); 
        return -1; 
    }

    char* token_name = argv[1]; 
    ACE_Remote_Mutex mutex(token_name, 0, 1); 
    ACE_TSS_Connection::set_server_address(ACE_INET_Addr(10202, ACE_LOCALHOST)); 

    int result = mutex.acquire(); 
    if(result == 0)
    {
        ACE_DEBUG((LM_DEBUG, "(%P/%t) acquire mutex!\n")); 
        ACE_OS::sleep(10); 
        mutex.release(); 
    }
    else 
        ACE_DEBUG((LM_DEBUG, "%p\n", "acquire lock")); 

	return 0;
}

//
//int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
//{
//    if(argc < 3)
//    {
//        ACE_DEBUG((LM_DEBUG, "need token names!\n")); 
//        return -1; 
//    }
//
//    char* token_name1 = argv[1]; 
//    char* token_name2 = argv[2]; 
//    ACE_Remote_Mutex mutex1(token_name1, 0, 1); 
//    ACE_Remote_Mutex mutex2(token_name2, 0, 1); 
//    ACE_TSS_Connection::set_server_address(ACE_INET_Addr(10202, ACE_LOCALHOST)); 
//    
//    while(1)
//    {
//        int result = mutex1.acquire(); 
//        if(result == 0)
//        {
//            ACE_DEBUG((LM_DEBUG, "(%P/%t) acquire mutex1!\n")); 
//            //ACE_OS::sleep(2); 
//            ACE_OS::sleep(ACE_Time_Value(0, 100000)); 
//            result = mutex2.acquire(); 
//            if(result == 0)
//            {
//                ACE_DEBUG((LM_DEBUG, "(%P/%t) acquire mutex2!\n")); 
//                //ACE_OS::sleep(5); 
//                mutex2.release(); 
//            }
//            else 
//                ACE_DEBUG((LM_DEBUG, "%p\n", "acquire lock")); 
//
//            mutex1.release(); 
//        }
//        else 
//            ACE_DEBUG((LM_DEBUG, "%p\n", "acquire lock")); 
//    }
//
//	return 0;
//}

