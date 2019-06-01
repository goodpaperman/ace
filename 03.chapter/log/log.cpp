// log.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/signal.h" 
//#include "time.h" 
#include "ace/OS_NS_sys_time.h"
#include "ace/OS_NS_time.h"
#include "trace.h" 
#include <iostream> 
#include <fstream> 


#if 0
#define DEBUG_PREFIX ACE_TEXT("DEBUG%I") 
#define INFO_PREFIX ACE_TEXT("INFO%I") 
#define NOTICE_PREFIX ACE_TEXT("NOTICE%I") 
#define WARNING_PREFIX ACE_TEXT("WARNING%I") 
#define ERROR_PREFIX ACE_TEXT("ERROR%I") 
#define CRITICAL_PREFIX ACE_TEXT("CRITICAL%I") 
#define ALERT_PREFIX ACE_TEXT("ALERT%I") 
#define EMERGENCY_PREFIX ACE_TEXT("EMERGENCY%I") 

#define MY_DEBUG(FMT, ...) \
  ACE_DEBUG((LM_DEBUG, DEBUG_PREFIX FMT __VA_ARGS__)) 

#define MY_INFO(FMT, ...) \
  ACE_DEBUG((LM_INFO, INFO_PREFIX FMT __VA_ARGS__)) 

#define MY_NOTICE(FMT, ...) \
  ACE_DEBUG((LM_NOTICE, NOTICE_PREFIX FMT __VA_ARGS__)) 

#define MY_WARNING(FMT, ...) \
  ACE_DEBUG((LM_WARNING, WARNING_PREFIX FMT __VA_ARGS__)) 

#define MY_ERROR(FMT, ...) \
  ACE_DEBUG((LM_ERROR, ERROR_PREFIX FMT __VA_ARGS__)) 

#define MY_CRITICAL(FMT, ...) \
  ACE_DEBUG((LM_CRITICAL, CRITICAL_PREFIX FMT __VA_ARGS__)) 

#define MY_ALERT(FMT, ...) \
  ACE_DEBUG((LM_ALERT, ALERT_PREFIX FMT __VA_ARGS__)) 

#define MY_EMERGENCY(FMT, ...) \
  ACE_DEBUG((LM_EMERGENCY, EMERGENCY_PREFIX FMT __VA_ARGS__)) 

#elif 0

#define MY_DEBUG LM_DEBUG, ACE_TEXT("DEBUG%I") 
#define MY_INFO LM_INFO, ACE_TEXT("INFO%I") 
#define MY_NOTICE LM_NOTICE, ACE_TEXT("NOTICE%I") 
#define MY_WARNING LM_WARNING, ACE_TEXT("WARNING%I") 
#define MY_ERROR LM_ERROR, ACE_TEXT("ERROR%I") 
#define MY_CRITICAL LM_CRITICAL, ACE_TEXT("CRITICAL%I") 
#define MY_ALERT LM_ALERT, ACE_TEXT("ALERT%I") 
#define MY_EMERGENCY LM_EMERGENCY, ACE_TEXT("EMERGENCY%I") 

#else 

#define MY_DEBUG LM_DEBUG, ACE_TEXT("%M%I") 
#define MY_INFO LM_INFO, ACE_TEXT("%M%I") 
#define MY_NOTICE LM_NOTICE, ACE_TEXT("%M%I") 
#define MY_WARNING LM_WARNING, ACE_TEXT("%M%I") 
#define MY_ERROR LM_ERROR, ACE_TEXT("%M%I") 
#define MY_CRITICAL LM_CRITICAL, ACE_TEXT("%M%I") 
#define MY_ALERT LM_ALERT, ACE_TEXT("%M%I") 
#define MY_EMERGENCY LM_EMERGENCY, ACE_TEXT("%M%I") 

#endif 


void foo(void); 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::STDERR); 
  //ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR); 
  //ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::SYSLOG | ACE_Log_Msg::STDERR, "syslogTest"); 

  //ACE_OSTREAM_TYPE* output = &std::cout; 
  //ACE_LOG_MSG->msg_ostream(output, 0); 

  //ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::STDERR | ACE_Log_Msg::OSTREAM); 
  ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::STDERR); 

  //ACE_TRACE(ACE_TEXT("main")); 
  TRACE("main"); 
  printf("log error = 0x%08x.\n", GetLastError()); 

  ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM); 
  ACE_OSTREAM_TYPE* output = new std::ofstream("ostream.output.test"); 
  ACE_LOG_MSG->msg_ostream(output, 1); 

  //ACE_LOG_MSG->priority_mask(LM_DEBUG | LM_NOTICE/* | LM_TRACE*/, ACE_Log_Msg::PROCESS); 
  //ACE_LOG_MSG->priority_mask(LM_ERROR, ACE_Log_Msg::PROCESS); 
  //ACE_LOG_MSG->priority_mask(LM_NOTICE, ACE_Log_Msg::THREAD); 
  //ACE_Log_Msg::enable_debug_messages(); 

  //ACE_DEBUG((LM_INFO, ACE_TEXT("%IHi Mom\n"))); 
  //MY_INFO(ACE_TEXT("Hi Mom\n")); 
  ACE_DEBUG((MY_INFO ACE_TEXT("Hi Mom\n"))); 
  foo(); 
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IGoodnight\n"))); 
  //MY_DEBUG(ACE_TEXT("Goodnight\n")); 
  ACE_DEBUG((MY_DEBUG ACE_TEXT("Goodnight\n")));

  char buf[32] = { "Hello World, Ni Hao Ma?" }; 
  ACE_HEX_DUMP((LM_NOTICE, buf, sizeof(buf), ACE_TEXT("HEX DUMP: "))); 


  //ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR); 
	//return 0;
  //ACE_LOG_MSG->open(argv[0]); 
  TRACE_RETURN(0); 
}

void display()
{
  //ACE_DEBUG((LM_ERROR, "display an error.\n")); 
  //MY_ERROR(ACE_TEXT("display an error.\n")); 
  ACE_DEBUG((MY_ERROR ACE_TEXT("display an error.\n"))); 
}

void foo(void)
{
  //ACE_TRACE(ACE_TEXT("foo")); 
  TRACE("foo"); 
  //ACE_ERROR((LM_NOTICE, ACE_TEXT("%IHowdy Pardner\n"))); 
  //MY_NOTICE(ACE_TEXT("Howdy Pardner\n")); 
  ACE_DEBUG((MY_NOTICE ACE_TEXT("Howdy Pardner\n"))); 

  ACE_DEBUG((LM_INFO, "start format test: \n")); 

  ACE_Time_Value tv = ACE_OS::gettimeofday ();
  ACE_timer_t now = tv.sec() + (double)tv.usec() / ACE_ONE_SECOND_IN_USECS; 
  ACE_DEBUG((LM_INFO, "%%A = %A\n", now)); 
  //ACE_DEBUG((LM_INFO, "%%a = %a\n")); 
  ACE_DEBUG((LM_INFO, "%%c = %c\n", 'a')); 
  ACE_DEBUG((LM_INFO, "%%C = %C\n", "hello world")); 
  ACE_DEBUG((LM_INFO, "%%c = %c\n", L'a')); 
  ACE_DEBUG((LM_INFO, "%%C = %C\n", L"hello world")); 
  ACE_DEBUG((LM_INFO, "%%i/d = %d, %i\n", 1, 2)); 
  ACE_DEBUG((LM_INFO, "%%I = %Inihaoma?\n")); 

  double fval = 3.14e-8; 
  float fval2 = 3.14e-8; 
  ACE_DEBUG((LM_INFO, "%%e/E/f/F/g/G = %e, %E, %f, %F, %g, %G.\n", fval, fval, (float)fval, (float)fval, fval, fval)); 
  ACE_DEBUG((LM_INFO, "%%e/E = %e, %E.\n", fval, fval2)); 
  ACE_DEBUG((LM_INFO, "%%g/G = %g, %G.\n", fval, fval2)); 
  ACE_DEBUG((LM_INFO, "%%f/F = %f, %F.\n", fval, fval2)); 
  ACE_DEBUG((LM_INFO, "%%l = %l\n")); 
  ACE_DEBUG((LM_SHUTDOWN, "%%M = %M\n")); 
  ACE_DEBUG((LM_DEBUG, "%%M = %M\n")); 
  ACE_DEBUG((LM_INFO, "%%M = %M\n")); 
  ACE_DEBUG((LM_NOTICE, "%%M = %M\n")); 
  ACE_DEBUG((LM_WARNING, "%%M = %M\n")); 
  ACE_DEBUG((LM_STARTUP, "%%M = %M\n")); 
  ACE_DEBUG((LM_ERROR, "%%M = %M\n")); 
  ACE_DEBUG((LM_CRITICAL, "%%M = %M\n")); 
  ACE_DEBUG((LM_ALERT, "%%M = %M\n")); 
  ACE_DEBUG((LM_EMERGENCY, "%%M = %M\n")); 

  errno = 1; 
  ACE_DEBUG((LM_INFO, "%%m = %m\n")); 
  ACE_DEBUG((LM_INFO, "%%N = %N\n")); 
  ACE_DEBUG((LM_INFO, "%%n = %n\n")); 
  ACE_DEBUG((LM_INFO, "%%o = %o\n", 11)); 
  ACE_DEBUG((LM_INFO, "%%P = %P\n")); 
  ACE_DEBUG((LM_INFO, "%%p = %p\n", ACE_TEXT("hello world"))); 

  ACE_UINT64 q = 1234567890987654321; 
  ACE_DEBUG((LM_INFO, "%%Q = %Q\n", q)); 
  ACE_DEBUG((LM_INFO, "%%r = %r\n", display)); 
  ACE_DEBUG((LM_INFO, "%%R = %R\n", 1234)); 
  ACE_DEBUG((LM_INFO, "%%S = %S\n", SIG_IGN)); 
  ACE_DEBUG((LM_INFO, "%%s = %s\n", ACE_TEXT("good citizen"))); 
  ACE_DEBUG((LM_INFO, "%%s = %s\n", "good citizen")); 
  ACE_DEBUG((LM_INFO, "%%s = %s\n", L"good citizen")); 
  ACE_DEBUG((LM_INFO, "%%T = %T\n")); 
  ACE_DEBUG((LM_INFO, "%%D = %D\n")); 
  ACE_DEBUG((LM_INFO, "%%t = %t\n")); 
  ACE_DEBUG((LM_INFO, "%%u = %u\n", -1)); 
  ACE_DEBUG((LM_INFO, "%%w = %w\n", L'a')); 
  ACE_DEBUG((LM_INFO, "%%W = %W\n", L"Hello World")); 
  ACE_DEBUG((LM_INFO, "%%w = %w\n", 'a')); 
  ACE_DEBUG((LM_INFO, "%%W = %W\n", "Hello World")); 
  ACE_DEBUG((LM_INFO, "\n%%x/X = %x, %X\n", &q, &q)); 
  ACE_DEBUG((LM_INFO, "%%@ = %@\n", &q)); 

  ACE_DEBUG((LM_INFO, "end format test.\n")); 
  ACE_ASSERT(1); 
  //ACE_ASSERT(0); 

  TRACE_NORETURN(); 
}

