#pragma once

#include "ace/Event_Handler.h" 

class timer_handler : public ACE_Event_Handler
{
public:
  timer_handler(void);
  ~timer_handler(void);

  void timer_id(long id) { id_ = id; } 
  long timer_id() const { return id_; } 

  virtual int handle_timeout(ACE_Time_Value const& current, void const* arg = 0); 

private:
  long id_; 
  int count_; 
};
