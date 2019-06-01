// invoke.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Thread_Manager.h" 
#include "ace/task.h"

ACE_thread_key_t main_key = 0; 
void cleanup_main_key(void *key)
{
  printf("key #%p (%u) cleaned up.\n", key, (*(int*)key)); 
  delete (int *)key; 
}

void cleanup_obj(void *obj, void *param)
{
  printf("obj #%p (%u) cleaned up.\n", obj, (*(int*)obj)); 
  delete (int *)obj; 
}

ACE_THR_FUNC_RETURN func(void* param)
{
  int *value = new int(300); 
  ACE_Thread::setspecific(main_key, value); 
  printf("set sepcific of #%p.\n", value); 

  int *obj = new int(123); 
  ACE_Thread_Manager::instance()->at_exit(obj, cleanup_obj, 0); 
  printf("obj #%p registered.\n", obj); 

  ACE_Thread::exit(2); 
  return 0; 
}

ACE_THR_FUNC_RETURN proc(void* param)
{
  int *value = new int(400); 
  ACE_Thread::setspecific(main_key, value); 
  printf("set sepcific of #%p.\n", value); 

  ACE_Thread::exit(2); 
  return 0; 
}

class our_task : public ACE_Task<ACE_MT_SYNCH>
{
public:
  virtual int svc()
  {
    int *value = new int(200); 
    ACE_Thread::setspecific(main_key, value);  
    printf("set sepcific of #%p.\n", value); 
    //ACE_Thread::exit(1); 
    return 0; 
  }
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int *value = new int(100); 
  ACE_Thread::keycreate(&main_key, &cleanup_main_key); 
  ACE_Thread::setspecific(main_key, value);  
  printf("set sepcific of #%p.\n", value); 

  our_task task; 
  task.activate(); 
  task.wait(); 

  ACE_thread_t t_id = 0; 
  ACE_Thread_Manager::instance()->spawn(func, 0, 0, &t_id); 
  //ACE_Thread_Manager::instance()->wait(); 
  ACE_Thread_Manager::instance()->join(t_id); 

  ACE_hthread_t t_handle = 0; 
  ACE_Thread::spawn(proc, 0, 0, 0, &t_handle); 
  ACE_Thread::join(t_handle); 

  //ACE_Thread::keyfree(main_key); 
  //cleanup_main_key(value); 
	return 0;
}

