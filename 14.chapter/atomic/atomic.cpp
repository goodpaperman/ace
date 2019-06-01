// atomic.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Atomic_Op.h" 
#include "ace/Task.h" 

#define MAX_PROD 50000
#define Q_SIZE 500

typedef ACE_Atomic_Op<ACE_Thread_Mutex, /*unsigned */long> SafeUInt; 
typedef ACE_Atomic_Op<ACE_Thread_Mutex, long> SafeInt; 

class Producer : public ACE_Task_Base 
{
public:
  Producer(int *buf, SafeUInt &in, SafeUInt &out)
    : buf_(buf), in_(in), out_(out)
  {
  }

  virtual int svc()
  {
    int item_num = 0; 
    while(item_num < MAX_PROD)
    {
      do
      {
      }while(in_.value() - out_.value() == Q_SIZE); 

      item_num++; 
      buf_[in_.value() % Q_SIZE] = item_num; 
      in_ ++; 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Produced %u\n"), item_num)); 
    }
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Total Produced %u\n"), item_num)); 
    return 0; 
  }

private:
  int *buf_; 
  SafeUInt &in_; 
  SafeUInt &out_; 
}; 

class Consumer : public ACE_Task_Base
{
public:
  Consumer(int *buf, SafeUInt &in, SafeUInt &out)
    : buf_(buf), in_(in), out_(out)
  {
  }

  virtual int svc()
  {
    int item_num = 0; 
    while(item_num < MAX_PROD)
    {
      do
      {
      }while(in_.value() - out_.value() == 0); 

      item_num = buf_[out_.value()%Q_SIZE]; 
      out_ ++; 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Consumed %u\n"), item_num)); 
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Total Consumed %u\n"), item_num)); 
    return 0; 
  }

private:
  int *buf_; 
  SafeUInt &in_; 
  SafeUInt &out_; 
}; 



int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int shared_buf[Q_SIZE] = { 0 }; 
  SafeUInt in = 0; 
  SafeUInt out = 0; 

  Producer p(shared_buf, in, out); 
  Consumer c(shared_buf, in, out); 

  p.activate(THR_NEW_LWP|THR_JOINABLE, 1, 1, ACE_THR_PRI_OTHER_MAX); 
  c.activate(THR_NEW_LWP|THR_JOINABLE, 1, 1, ACE_THR_PRI_OTHER_MIN); 

  p.wait(); 
  c.wait(); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("in: %u, out: %u\n"), in.value(), out.value())); 

	return 0;
}

