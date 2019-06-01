// memstream.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/INET_Addr.h" 
#include "ace/MEM_Connector.h" 
#include "ace/MEM_Stream.h" 
#include "ace/os_ns_netdb.h" 

#define USE_MT_IMPL

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_MEM_Stream stream; 
  ACE_MEM_Connector connector; 
  ACE_TCHAR name[MAXHOSTNAMELEN + 1];
  if (ACE_OS::hostname (name, MAXHOSTNAMELEN+1) == -1)
    return -1;

  ACE_INET_Addr remote_addr(1314, name); 
#if defined(USE_MT_IMPL)
  connector.preferred_strategy(ACE_MEM_IO::MT); 
#endif 

  if(connector.connect(stream, remote_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("connect")), -1); 

  const int buflen = 1024; 
  char buf[buflen] = { 0 };
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connect to server: [OK]\n"))); 
  while(1)
  {
    if(stream.send(ACE_TEXT("How are you ?"), 14) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send")), -1); 

    if(stream.recv(buf, buflen) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("recv")), -1); 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv: %s\n"), buf)); 
  }

	return 0;
}

