// logex.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/Log_Msg_Callback.h" 
#include "ace/Log_Record.h" 
#include "ace/streams.h"
#include "ace/SString.h" 
#include "ace/os_ns_time.h" 

#define USE_CALLBACK
#define CALLBACK_EX

#ifdef USE_CALLBACK 
class Callback : public ACE_Log_Msg_Callback
{
public:
  void log(ACE_Log_Record& log_record)
  {
#  ifdef CALLBACK_EX
    cerr << "Log Message Received: " << endl; 
    ACE_Log_Priority prio = ACE_static_cast(ACE_Log_Priority, log_record.type()); 
    ACE_TCHAR const* prio_name = ACE_Log_Record::priority_name(prio); 
    cerr << "\tType: " << ACE_TEXT_ALWAYS_CHAR(prio_name) << endl; 
    cerr << "\tLength: " << log_record.length() << endl; 
    time_t const epoch = log_record.time_stamp().sec(); 
    cerr << "\tTime_Stamp: " << ACE_TEXT_ALWAYS_CHAR(ACE_OS::ctime(&epoch)) << flush; 
    cerr << "\tPid: " << log_record.pid() << endl; 

    ACE_CString data(">> "); 
    data += ACE_TEXT_ALWAYS_CHAR(log_record.msg_data()); 
    cerr << "\tMsgData: " << data.c_str() << endl; 
#  else 
    log_record.print(ACE_TEXT("localhost"), 0, cerr); 
    log_record.print(ACE_TEXT("localhost"), ACE_Log_Msg::VERBOSE_LITE, cerr); 
    log_record.print(ACE_TEXT("localhost"), ACE_Log_Msg::VERBOSE, cerr); 
#  endif 
  }
}; 
#endif 

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef USE_CALLBACK
  Callback* callback = new Callback(); 
  ACE_LOG_MSG->msg_callback(callback); 
  ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::MSG_CALLBACK); 
  ACE_TRACE(ACE_TEXT("main")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IHi Mom\n"))); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%IGoodnight\n"))); 
#else
  ACE_LOG_MSG->open(argv[0]); 
  ACE_TRACE(ACE_TEXT("main"));
  ACE_OSTREAM_TYPE* output = new std::ofstream("ostream.output.test"); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IThis will go to STDERR\n"))); 
  ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::SYSLOG, ACE_TEXT("syslogTest")); 
  ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IThis goes to STDERR & syslog\n"))); 

  ACE_LOG_MSG->msg_ostream(output, 0); 
  ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%IThis will go to STDERR, syslog & an ostream\n"))); 
  
  //ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM); 
  ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM); 
  delete output; 
#endif 

	return 0;
}

