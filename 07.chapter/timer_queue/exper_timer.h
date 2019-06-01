#pragma once

#include "ace/Reactor.h" 

class temp_sensor
{
public:
  temp_sensor(char const* location) 
    : timer_(-1)
    , interval_(1)
    , location_(location)
    , count_(0)
    , totalplus_(0.0)
    , current_temp_(0.0)
  {
  }

  void interval(int val) { interval_ = val; } 
  int interval() const { return interval_; } 
  void timer(int val) { timer_ = val; } 
  int timer() const { return timer_; } 
  char const* location() const { return location_; } 
  float current_temp() const { return current_temp_; } 
  float avg_temp() const { return count_ > 0 ? totalplus_ / count_ : 0; } 

  int query_sensor()
  {
    current_temp_ = 20 + rand() % 20; 
    totalplus_ += current_temp_; 
    ++ count_; 
    return 0; 
  }

private:
  int timer_; 
  int interval_; 
  char const* location_; 
  int count_; 
  float totalplus_; 
  float current_temp_; 
}; 

class exper_timer : public ACE_Event_Handler
{
public:
  exper_timer(ACE_Reactor* r = ACE_Reactor::instance());
  virtual ~exper_timer(void);

  virtual int handle_timeout(ACE_Time_Value const& current_time, void const* act = 0); 
  virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask); 

private:
  int timer1_; 
  int timer2_; 
};
