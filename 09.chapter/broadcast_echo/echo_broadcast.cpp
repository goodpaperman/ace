// echo_broadcast.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Log_Msg.h" 
#include "ace/SOCK_Dgram_Bcast.h" 
#include "ace/INET_Addr.h" 
#include "ace/Signal.h" 

int _running = 1; 
void handle_signal(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}

int echo_broadcast(ACE_SOCK_Dgram_Bcast &udp)
{
  ACE_INET_Addr from; 
  char addrstr[128] = { 0 }; 
  char buf[1024] = { 0 }; 
  int buflen = 1024; 

  ssize_t recv = udp.recv(buf, buflen, from); 
  if(recv == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("recv")), -1); 
  else 
  {
    from.addr_to_string(addrstr, 128); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%s) recv: [%d] %s"), addrstr, recv, buf)); 
  }

  //ssize_t sent = udp.send(buf, recv, from.get_port_number()); 
  ssize_t sent = udp.send(buf, recv, from); 
  if(sent == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("recv")), -1); 
  else 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send: %d\n"), sent)); 

  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_INET_Addr my_addr(10102); 
  //ACE_INET_Addr my_addr(10102, "localhost"); 
  ACE_SOCK_Dgram_Bcast udp(my_addr); 

  ACE_Sig_Action sigact(handle_signal, SIGINT); 
  while(_running)
  {
    echo_broadcast(udp); 
  }

	return 0;
}

