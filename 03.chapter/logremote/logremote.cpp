// logremote.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 

#define LOG_DIRECTLY

#ifdef LOG_DIRECTLY
#include "callback.h" 
#endif 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#ifdef LOG_DIRECTLY
  Callback *callback = new Callback(); 
  ACE_LOG_MSG->msg_callback(callback); 
  ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::MSG_CALLBACK | ACE_Log_Msg::STDERR); 
#else 
  ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::LOGGER | ACE_Log_Msg::STDERR, ACE_DEFAULT_LOGGER_KEY); 
#endif 

  ACE_TRACE(ACE_TEXT("main")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IHi Mom\n"))); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IGoodnight\n"))); 

#ifdef LOG_DIRECTLY
  // cleanup the callback
  ACE_LOG_MSG->msg_callback(0); 
  delete callback; 
#endif 
	return 0;
}

