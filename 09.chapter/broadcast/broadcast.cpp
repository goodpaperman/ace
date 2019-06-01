// broadcast.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Log_Msg.h" 
#include "ace/SOCK_Dgram_Bcast.h" 
#include "ace/INET_Addr.h" 
#include "ace/Signal.h" 


int send_broadcast(ACE_SOCK_Dgram_Bcast &udp, u_short to)
{
  char const* msg = "this is the message!\n"; 
  ssize_t sent = udp.send(msg, strlen(msg)+1, to); 

  if(sent == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("send")), -1); 
  else 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send: %d\n"), sent)); 

  return 0; 
}

int recv_broadcast(ACE_SOCK_Dgram_Bcast &udp, ACE_INET_Addr &from)
{
  char buf[1024] = { 0 }; 
  int buflen = 1024; 
  ssize_t recv = udp.recv(buf, buflen, from); 

  if(recv == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("recv")), -1); 
  else 
  {
    char addrstr[128] = { 0 }; 
    from.addr_to_string(addrstr, 128); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%s) recv: [%d] %s"), addrstr, recv, buf)); 
  }

  return 0; 
}

int _running = 1; 
void handle_signal(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //ACE_INET_Addr my_addr; 
  ACE_INET_Addr my_addr(10101); 
  //ACE_INET_Addr my_addr(10101, "localhost"); 

  //ACE_SOCK_Dgram_Bcast udp(ACE_Addr::sap_any); 
  ACE_SOCK_Dgram_Bcast udp(my_addr); 

  ACE_Sig_Action sigact(handle_signal, SIGINT); 
  ACE_INET_Addr from; 
  while(_running)
  {
    send_broadcast(udp, 10102); 
    recv_broadcast(udp, from); 
  }

  udp.close(); 
	return 0;
}

