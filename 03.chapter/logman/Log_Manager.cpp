#include "StdAfx.h"
#include "Log_Manager.h"

Log_Manager::Log_Manager(void)
: log_stream_(0)
//, output_stream_(0) 
{
}

Log_Manager::~Log_Manager(void)
{
  if(log_stream_)
  {
    log_stream_->close(); 
    delete log_stream_; 
  }

  //output_stream_ = 0; 
  //if(output_stream_)
  //{
  //  output_stream_->close(); 
  //  delete output_stream_; 
  //}
}

int Log_Manager::redirectToDaemon(ACE_TCHAR const* prog_name /*= ACE_TEXT("")*/)
{
  ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::OSTREAM | ACE_Log_Msg::SYSLOG | ACE_Log_Msg::MSG_CALLBACK); 
  return ACE_LOG_MSG->open(prog_name, ACE_Log_Msg::LOGGER, ACE_DEFAULT_LOGGER_KEY); 
}


int Log_Manager::redirectToSyslog(ACE_TCHAR const* prog_name /*= ACE_TEXT("")*/)
{
  ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::OSTREAM | ACE_Log_Msg::LOGGER | ACE_Log_Msg::MSG_CALLBACK); 
  return ACE_LOG_MSG->open(prog_name, ACE_Log_Msg::SYSLOG, prog_name); 
}

void Log_Manager::redirectToOStream(ACE_OSTREAM_TYPE* output)
{
  ACE_LOG_MSG->msg_ostream(output); 
  ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::LOGGER | ACE_Log_Msg::SYSLOG | ACE_Log_Msg::MSG_CALLBACK); 
  ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM); 
}

void Log_Manager::redirectToFile(char const* filename)
{
  if(log_stream_)
  {
    log_stream_->close(); 
    delete log_stream_; 
  }

  log_stream_ = new ofstream(); 
  log_stream_->open(filename, ios::out | ios::app); 
  this->redirectToOStream(log_stream_); 
}

void Log_Manager::redirectToStderr(void)
{
  ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM | ACE_Log_Msg::LOGGER | ACE_Log_Msg::SYSLOG | ACE_Log_Msg::MSG_CALLBACK); 
  ACE_LOG_MSG->set_flags(ACE_Log_Msg::STDERR); 
}

ACE_Log_Msg_Callback* Log_Manager::redirectToCallback(ACE_Log_Msg_Callback* callback)
{
  ACE_Log_Msg_Callback* previous = ACE_LOG_MSG->msg_callback(callback); 
  if(callback == 0)
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::MSG_CALLBACK); 
  else 
  {
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM | ACE_Log_Msg::LOGGER | ACE_Log_Msg::SYSLOG | ACE_Log_Msg::STDERR); 
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::MSG_CALLBACK); 
  }

  return 0; 
}


#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
  template class ACE_Singleton<Log_Manager, ACE_Null_Mutex>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#  pragma instantiate ACE_Singleton<Log_Manager, ACE_Null_Mutex>
#elif defined(__GNUC__) && (defined(_AIX) || defined(__hpux))
  template ACE_Singleton<Log_Manager, ACE_Null_Mutex>* ACE_Singleton<Log_Manager, ACE_Null_Mutex>::singleton_; 
#endif 
