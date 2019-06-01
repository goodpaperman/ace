// memap.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "ace/ACE.h"
#include "ace/Mem_Map.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_String.h" 
#include "ace/OS_NS_unistd.h" 

#define PROTECTOR

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    int page = ACE_OS::allocation_granularity (); 
    ACE_Mem_Map map ("c:/file.map", 1024 * 1024, O_CREAT | O_RDWR, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, MAP_SHARED); 
    //ACE_Mem_Map map ("c:/file.map", 2 * 1024 * 1024, O_CREAT | O_RDWR, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, MAP_SHARED); 
    //ACE_Mem_Map map ("c:/file.map", -1, O_CREAT | O_RDWR, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, MAP_SHARED); 
    //ACE_Mem_Map map ("c:/file.map", 1024*1024, O_CREAT | O_RDWR, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, MAP_SHARED | MAP_FIXED, (void *)0x40000000); 
    //ACE_Mem_Map map ("c:/file.map", 1024*1024 - page, O_CREAT | O_RDWR, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, MAP_SHARED | MAP_FIXED, (void *)0x40000000, page); 

    if (map.addr () != MAP_FAILED)
    {
        ACE_OS::strcpy ((char *)map.addr (), "hello world !"); 
        ACE_DEBUG ((LM_DEBUG, "string = (0x%08x)%s\n", map.addr (), map.addr ())); 
        
        map.map (2 * 1024 * 1024, PROT_RDWR, MAP_SHARED); 
        if (map.addr () != MAP_FAILED)
        {
            ACE_OS::strcpy ((char *)map.addr (), "hi world !"); 
            ACE_DEBUG ((LM_DEBUG, "string = (0x%08x)%s\n", map.addr (), map.addr ())); 
        }
#if 0
#if defined (PROTECTOR)
        int ret = map.protect (map.addr (), page, PROT_READ); 
        ACE_DEBUG ((LM_DEBUG, "%T protect (0, %d) = %d\n", page, ret)); 
        // this statement will cause access exception.
        ACE_OS::strcpy ((char *)map.addr (), "world wild"); 
        ACE_OS::sleep (10); 
        ACE_DEBUG ((LM_DEBUG, "%T unprotect (0, %d)\n", page)); 
        map.protect (map.addr (), page, PROT_WRITE); 
#else 
        ACE_OS::sleep (2); 
        ACE_DEBUG ((LM_DEBUG, "string = (0x%08x)%s\n", map.addr (), map.addr ())); 
        ACE_OS::sleep (2); 
        // this statement will NOT cause exception.
        ACE_OS::strcpy ((char *)map.addr (), "hi world!"); 
        ACE_DEBUG ((LM_DEBUG, "string = (0x%08x)%s\n", map.addr (), map.addr ())); 
        ACE_OS::sleep (2); 
#endif 
#endif 

        map.sync (); 
        map.advise (1); 
        //map.remove (); 
    }

	return 0;
}

