#pragma once

#include "ace/Thread_Hook.h" 
#include "ace/tss_t.h" 

class Noop
{
public:
  Noop(); 
  ~Noop(); 

  int this_is; 
  int a; 
  int no; 
  int op; 
}; 

class HA_ThreadHook : public ACE_Thread_Hook
{
public:
  virtual ACE_THR_FUNC_RETURN start(ACE_THR_FUNC func, void* arg); 

private:
  ACE_TSS<Noop> tss;
};
