// token.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HA_Device_Repository.h" 
#include "HA_CommandHandler.h" 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    HA_Device_Repository rep; 
    HA_CommandHandler handler(rep); 
    handler.activate(THR_NEW_LWP | THR_JOINABLE, HA_CommandHandler::N_THREADS); 
    handler.wait(); 

    //ACE_OS::sleep(10); 
	return 0;
}

