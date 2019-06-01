// spipe_echo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/SPIPE_Stream.h" 
#include "ace/SPIPE_Acceptor.h" 
#include "ace/SPIPE_Addr.h" 
#include "ace/Log_Msg.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_SPIPE_Acceptor acceptor; 
  ACE_SPIPE_Addr local_addr("spipe"), remote_addr; 
  //ACE_SPIPE_Addr local_addr("7788"), remote_addr; 
  if(acceptor.open(local_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("open")), -1); 

  int recv = 0; 
  int const buflen = 1024; 
  char buf[buflen] = { 0 }; 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("open acceptor: [OK]\n"))); 
  while(1)
  {
    ACE_SPIPE_Stream stream; 
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

