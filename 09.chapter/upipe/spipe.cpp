// spipe.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/UPIPE_Addr.h" 
#include "ace/UPIPE_Stream.h" 
#include "ace/UPIPE_Connector.h" 
#include "ace/UPIPE_Acceptor.h" 
#include "ace/Log_Msg.h"
#include "ace/signal.h" 

int _running = 1; 
void sighandler(int)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}

ACE_THR_FUNC_RETURN accept_proc(void* arg)
{
  ACE_DEBUG((LM_TRACE, ACE_TEXT("server start.\n"))); 
  ACE_UPIPE_Acceptor acceptor; 
  ACE_UPIPE_Addr local_addr("upipe"), remote_addr; 
  if(acceptor.open(local_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("open")), -1); 

  int recv = 0; 
  int const buflen = 1024; 
  char buf[buflen] = { 0 }; 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("open acceptor: [OK]\n"))); 
  while(_running)
  {
    ACE_UPIPE_Stream stream; 
    if(acceptor.accept(stream, &remote_addr) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("accept")), -1); 

    while(_running)
    {
      if((recv = stream.recv(buf,  buflen)) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("recv"))); 
        break; 
      }

      buf[recv] = 0; 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("server recv: %s\n"), buf)); 
      if(stream.send(buf, recv) == -1)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p"), ACE_TEXT("send"))); 
        break; 
      }
    }

    stream.close(); 
  }

  ACE_DEBUG((LM_TRACE, ACE_TEXT("server exit.\n"))); 
	return 0;
}

ACE_THR_FUNC_RETURN connect_proc(void* arg)
{
  ACE_DEBUG((LM_TRACE, ACE_TEXT("client start.\n"))); 
  ACE_UPIPE_Stream stream; 
  ACE_UPIPE_Connector connector; 
  ACE_UPIPE_Addr remote_addr("upipe"); 
  if(connector.connect(stream, remote_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("connect")), -1); 

  const int buflen = 1024; 
  char buf[buflen] = { 0 };
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connect to server: [OK]\n"))); 
  while(_running)
  {
    if(stream.send(ACE_TEXT("How are you ?"), 14) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send")), -1); 

    if(stream.recv(buf, buflen) == -1)
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("recv")), -1); 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("client recv: %s\n"), buf)); 
  }

  stream.close(); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("client exit.\n"))); 
  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_Sig_Action sigact(sighandler, SIGINT); 
  if(ACE_Thread_Manager::instance()->spawn(accept_proc) == -1)
    return -1; 

  ACE_OS::sleep(1); 
  if(ACE_Thread_Manager::instance()->spawn(connect_proc) == -1)
    return -1; 

  ACE_Thread_Manager::instance()->wait(); 
  return 0; 
}

