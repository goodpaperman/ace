// spipe.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Multihomed_INET_Addr.h" 
#include "ace/SOCK_SEQPACK_Association.h" 
#include "ace/SOCK_SEQPACK_Connector.h" 
#include "ace/Log_Msg.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_SOCK_SEQPACK_Association stream; 
  ACE_SOCK_SEQPACK_Connector connector; 
  ACE_Multihomed_INET_Addr remote_addr(7788); 
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

