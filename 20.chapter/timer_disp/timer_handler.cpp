#include "StdAfx.h"
#include "timer_handler.h"
#include "timer_dispather.h" 

timer_handler::timer_handler(void)
: id_(-1)
, count_(0) 
{
}

timer_handler::~timer_handler(void)
{
}

int timer_handler::handle_timeout(ACE_Time_Value const& current, void const* arg /* =0 */)
{
  ACE_TRACE(ACE_TEXT("timer_handler::handle_timeout")); 
  int const* val = (int const*)arg; 
  ACE_ASSERT(*val == id_); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("timeout: %d (%d).\n"), id_, count_)); 
  if(count_ == 3)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("reseting interval for timer %d\n"), id_)); 
    ACE_Time_Value interval(0, 1000L); 
    ACE_ASSERT(TIMER::instance()->reset_interval(id_, interval) != -1); 
  }

  if(count_ == 10)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("canceling %d\n"), id_)); 
    ACE_ASSERT((TIMER::instance()->cancel(id_) != 0)); 
  }

  ++ count_; 
  return 0; 
}
