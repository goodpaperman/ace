#pragma once

class Trace
{
public:
  enum { nesting_ident_ = 3 }; 

  Trace(const ACE_TCHAR* prefix, 
    const ACE_TCHAR* name, 
    int line, 
    const ACE_TCHAR* file)
    : prefix_(prefix)
    , name_(name)
    , line_(line)
    , file_(file) 
  {
    ACE_Log_Msg* lm = ACE_LOG_MSG; 
    if(lm->tracing_enabled() && 
      lm->trace_active() == 0)
    {
      lm->trace_active(1); 
      ACE_DEBUG((LM_TRACE, 
        ACE_TEXT("%s%*s(%t) enter %s: \"%s\" (%d)\n"), 
        this->prefix_, 
        Trace::nesting_ident_ * lm->inc(), 
        ACE_TEXT(""), 
        this->name_, 
        this->file_, 
        this->line_)); 
      lm->trace_active(0); 
    }
  }

  ~Trace(void)
  {
    ACE_Log_Msg* lm = ACE_LOG_MSG; 
    if(lm->tracing_enabled() && 
      lm->trace_active() == 0)
    {
      lm->trace_active(); 
      ACE_DEBUG((LM_TRACE, 
        ACE_TEXT("%s%*s(%t) leave %s: \"%s\" (%d)\n"), 
        this->prefix_, 
        Trace::nesting_ident_ * lm->dec(), 
        ACE_TEXT(""), 
        this->name_, 
        this->file_, 
        this->line_)); 
      lm->trace_active(0); 
    }
  }

  void set_line(int line) { this->line_ = line; } 

private: 
  ACE_TCHAR const* prefix_; 
  ACE_TCHAR const* name_; 
  ACE_TCHAR const* file_; 
  int line_; 
};


#define TRACE_PREFIX ACE_TEXT("TRACE ") 

#if (ACE_NTRACE == 1)
#  define TRACE(X)
#  define TRACE_RETURN(V) return V
#  define TRACE_NORETURN()
#else 
#  define TRACE(X) Trace _____ (TRACE_PREFIX, ACE_TEXT(X), __LINE__, ACE_TEXT(__FILE__)) 
#  define TRACE_RETURN(V) do { _____.set_line(__LINE__); return V; } while(0) 
#  define TRACE_NORETURN() do { _____.set_line(__LINE__); } while(0) 
#endif 