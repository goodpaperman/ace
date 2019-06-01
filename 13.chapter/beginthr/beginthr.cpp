// beginthr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#define USE_C_API

#if defined(USE_C_API)
unsigned 
#else 
DWORD
#endif 
WINAPI horse_work(void *arg)
{
  //errno = *(int *)arg; 
  //::Sleep(2000); 
  //printf("thread %u has error number %d\n", ::GetCurrentThreadId(), errno); 

  char* str = (char*)arg; 
  char* tok = strtok(str, "/"); 
  while(tok != 0)
  {
    printf("thread %u has token %s\n", ::GetCurrentThreadId(), tok); 
    tok = strtok(0, "/"); 
    Sleep(300); 
  }

  return 0; 
}

int _tmain(int argc, TCHAR* argv[])
{
#if defined(USE_C_API)
  unsigned thrid1 = 0; 
#else 
  DWORD thrid1 = 0; 
#endif 

  char str1[100] = "/home/haihai107/ACE5.6/lib"; 

#if defined(USE_C_API)
  HANDLE thr1 = (HANDLE)::_beginthreadex(0, 0, &horse_work, str1, 0, &thrid1); 
#else 
  HANDLE thr1 = ::CreateThread(0, 0, &horse_work, str1, 0, &thrid1); 
#endif 

  if(thr1 == INVALID_HANDLE_VALUE)
    return -1; 

  printf("create thread %u\n", thrid1); 

#if defined(USE_C_API)
  unsigned thrid2 = 0; 
#else 
  DWORD thrid2 = 0; 
#endif 

  char str2[100] = "/etc/sysconfig/network/scripts"; 

#if defined(USE_C_API)
  HANDLE thr2 = (HANDLE)::_beginthreadex(0, 0, &horse_work, str2, 0, &thrid2); 
#else 
  HANDLE thr2 = ::CreateThread(0, 0, &horse_work, str2, 0, &thrid2); 
#endif 

  if(thr2 == INVALID_HANDLE_VALUE)
    return -1; 

  printf("create thread %u\n", thrid2); 
  ::WaitForSingleObject(thr1, INFINITE); 
  ::WaitForSingleObject(thr2, INFINITE); 
  ::CloseHandle(thr1);   
  ::CloseHandle(thr2); 
	return 0;
}

