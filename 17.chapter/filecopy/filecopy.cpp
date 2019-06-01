// filecopy.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/MEM_Map.h" 
#include "ace/OS_NS_fcntl.h" 
#include "ace/OS_NS_String.h" 
#include "ace/Log_Msg.h" 
#include <algorithm> 

#define REVERT 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if (argc < 3)
        ACE_ERROR_RETURN ((LM_ERROR, "at least 2 parameters\n"), -1); 

    ACE_HANDLE src_handle = ACE_OS::open (argv[1], O_RDONLY); 
    if (src_handle == ACE_INVALID_HANDLE)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "open src file"), -1); 

    ACE_Mem_Map src_map (src_handle, -1, PROT_READ, ACE_MAP_PRIVATE); 
    if (src_map.addr () == MAP_FAILED)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "map src file"), -1); 

    ACE_Mem_Map dest_map (argv[2], src_map.size (), O_RDWR | O_CREAT, ACE_DEFAULT_FILE_PERMS, PROT_RDWR, ACE_MAP_SHARED); 
    if (dest_map.addr () == MAP_FAILED)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "map dest file"), -1); 

#if defined (REVERT)
    //int count = src_map.size (); 
    //for (int i=0; i<count; ++ i)
    //    ((char *)dest_map.addr ())[i] = ((char *)src_map.addr ())[count-1-i]; 
    std::reverse_copy ((char *)src_map.addr (), (char *)src_map.addr () + src_map.size (), (char *)dest_map.addr ()); 
#else 
    ACE_OS::memcpy (dest_map.addr (), src_map.addr (), src_map.size ()); 
#endif 

    dest_map.sync (); 
    src_map.close (); 
    dest_map.close (); 
	return 0;
}

