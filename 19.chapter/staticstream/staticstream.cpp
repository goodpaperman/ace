// staticstream.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Service_Config.h" 

//#include "HA_Stream.h"
//#include "HA_Encryptor.h"
//#include "HA_Compressor.h"
//#include "HA_Xmit.h" 

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    ACE_STATIC_SVC_REGISTER (HA_Stream); 
    ACE_STATIC_SVC_REGISTER (HA_Encryptor); 
    ACE_STATIC_SVC_REGISTER (HA_Compressor); 
    ACE_STATIC_SVC_REGISTER (HA_Xmit); 

    int ret = ACE_Service_Config::open (argc, argv, ACE_DEFAULT_LOGGER_KEY, 0); 
    if (ret != 0)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "Service_Config open"), -1); 

    ACE_Reactor::instance ()->run_reactor_event_loop (&ACE_Reactor::check_reconfiguration); 
	return 0;
}

