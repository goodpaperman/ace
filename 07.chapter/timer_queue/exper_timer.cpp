#include "StdAfx.h"
#include "exper_timer.h"
#include "ace/os_ns_time.h" 

exper_timer::exper_timer(ACE_Reactor *r /* =ACE_Reactor::instance() */)
: timer1_ (-1)
, timer2_ (-1) 
{
  reactor(r); 

  ACE_Time_Value init_delay(5); 
  ACE_Time_Value interval(3); 

  temp_sensor* s1 = new temp_sensor("Kitchen"); 
  s1->interval(3); 
  timer1_ = reactor()->schedule_timer(this, s1, init_delay, interval); 
  if(timer1_ == -1)
    delete s1; 
  else 
    s1->timer(timer1_); 
  
  interval.sec(5); 
  temp_sensor* s2 = new temp_sensor("Foyer"); 
  s2->interval(5); 
  timer2_ = reactor()->schedule_timer(this, s2, init_delay, interval); 
  if(timer2_ == -1)
    delete s2; 
  else 
    s2->timer(timer2_); 
}

exper_timer::~exper_timer(void)
{
  void const* arg = 0; 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove timers.\n"))); 
  if(timer1_ != -1 && reactor()->cancel_timer(timer1_, &arg) == 1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove timer1.\n"))); 
    delete (temp_sensor*)arg; 
    arg = 0; 
    timer1_ = -1; 
  }

  if(timer2_ != -1 && reactor()->cancel_timer(timer2_, &arg) == 1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("remove timer2.\n"))); 
    delete (temp_sensor*)arg; 
    arg = 0; 
    timer2_ = -1; 
  }
}

int exper_timer::handle_timeout(ACE_Time_Value const& current_time, void const* act /* =0 */)
{
  time_t epoch = ((timespec_t)current_time).tv_sec; 
  temp_sensor* sensor = (temp_sensor*)act; 
  sensor->query_sensor(); 

  //ACE_DEBUG((LM_INFO, ACE_TEXT("timeout : %s"), ACE_OS::ctime(&epoch))); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("%s, temperature = %.2f, avg = %.2f at %s"), 
    sensor->location(), 
    sensor->current_temp(), 
    sensor->avg_temp(), 
    ACE_OS::ctime(&epoch))); 
  
  if(sensor->interval() > 5)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recycle timer object.\n"))); 
    sensor->interval(0); 
    delete sensor; 
    return -1; // will call handle_close.
  }

  sensor->interval(sensor->interval() + 1); 
  reactor()->reset_timer_interval(sensor->timer(), sensor->interval()); 
  return 0; 
}

int exper_timer::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("close timer.\n"))); 

  //void const* arg = 0; 
  //if(timer1_ != -1 && reactor()->cancel_timer(timer1_, &arg) == 1)
  //{
  //  delete (temp_sensor*)arg; 
  //  arg = 0; 
  //  timer1_ = -1; 
  //}

  //if(timer2_ != -1 && reactor()->cancel_timer(timer2_, &arg) == 1)
  //{
  //  delete (temp_sensor*)arg; 
  //  arg = 0; 
  //  timer2_ = -1; 
  //}

  //delete this; 
  return 0; 
}