#pragma once

#include <fstream> 
#include "ace/Log_Msg.h" 
#include "ace/Singleton.h" 
//#include "ace/Guard_T.h"
#include "ace/Null_Mutex.h" 

using std::ofstream; 

class Log_Manager
{
public:
  Log_Manager(void);
  ~Log_Manager(void);

  int redirectToDaemon(ACE_TCHAR const* prog_name = ACE_TEXT("")); 
  int redirectToSyslog(ACE_TCHAR const* prog_name = ACE_TEXT("")); 
  void redirectToOStream(ACE_OSTREAM_TYPE* output); 
  void redirectToFile(char const* filename); 
  void redirectToStderr(void); 
  ACE_Log_Msg_Callback* redirectToCallback(ACE_Log_Msg_Callback* callback); 

private:
  ofstream* log_stream_; 
  //ACE_OSTREAM_TYPE* output_stream_; 
};

typedef ACE_Singleton<Log_Manager, ACE_Null_Mutex> Log_Manager_Singleton; 
#define LOG_MANAGER Log_Manager_Singleton::instance()
