// multicast.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Log_Msg.h" 
#include "ace/SOCK_Dgram_Mcast.h" 
#include "ace/INET_Addr.h" 
#include "ace/Signal.h" 

int _running = 1; 
void handle_signal(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("Ctrl+C pressed by user, exiting...\n"))); 
  _running = 0; 
}

int send_mcast(ACE_SOCK_Dgram_Mcast &udp)
{
  char* msg = "hello world!\n"; 
  ssize_t sent = udp.send(msg, strlen(msg)+1); 
  if(sent == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("send")), -1); 
  else
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("send: %d\n"), sent)); 

  return 0; 
}

int recv_mcast(ACE_SOCK_Dgram_Mcast &udp)
{
  char buf[1024] = { 0 }; 
  int buflen = 1024; 
  ACE_INET_Addr from; 
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

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  if(argc != 3)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("usage: \n\rmulticast.exe port mode\n")), -1); 
  }

  u_short port = atoi(argv[1]); 
  u_short mode = atoi(argv[2]); 

  //ACE_INET_Addr mcast_addr; 
  //ACE_INET_Addr mcast_addr(port); 
  ACE_INET_Addr mcast_addr(port, "localhost"); 
  //ACE_INET_Addr mcast_addr(port, "192.168.41.1"); 
  //ACE_INET_Addr mcast_addr(port, "192.168.145.1"); 

  ACE_SOCK_Dgram_Mcast udp; 
  //if(udp.open(mcast_addr, "192.168.0.1") == -1)
  if(udp.open(mcast_addr) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open")), -1); 

  //if(udp.join(mcast_addr) == -1)
  //  ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("join")), -1); 

  ACE_Sig_Action sigact(handle_signal, SIGINT); 
  while(_running)
  {
    if(mode == 1 || mode == 3)
      send_mcast(udp); 

    if(mode == 2 || mode == 3)
      recv_mcast(udp); 
  }

  udp.close(); 
	return 0;
}

