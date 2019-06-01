// spipe_echo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/SOCK_SEQPACK_Association.h" 
#include "ace/SOCK_SEQPACK_Acceptor.h" 
#include "ace/Multihomed_INET_Addr.h" 
#include "ace/Log_Msg.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_SOCK_SEQPACK_Acceptor acceptor; 
  ACE_Multihomed_INET_Addr local_addr(7788), remote_addr; 
  if(acceptor.open(local_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("open")), -1); 

  int recv = 0; 
  int const buflen = 1024; 
  char buf[buflen] = { 0 }; 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("open acceptor: [OK]\n"))); 
  while(1)
  {
    ACE_SOCK_SEQPACK_Association stream; 
    if(acceptor.accept(stream, &remote_addr) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("accept")), -1); 

    while(1)
    {
      if((recv = stream.recv(buf,  buflen)) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("recv"))); 
        break; 
      }

      buf[recv] = 0; 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv: %s\n"), buf)); 
      if(stream.send(buf, recv) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("send"))); 
        break; 
      }
    }

    stream.close(); 
  }

	return 0;
}

