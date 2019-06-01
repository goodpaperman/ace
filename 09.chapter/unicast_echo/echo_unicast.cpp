// echo_unicast.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ace/Log_Msg.h" 
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Dgram.h" 

int echo_dgram(ACE_SOCK_Dgram &udp)
{
  ACE_INET_Addr your_addr; 
  char buf[1024] = { 0 }; 
  int buflen = 1024; 

  ssize_t recv = udp.recv(buf, buflen, your_addr); 
  if(recv > 0)
  {
    char addrstr[128] = { 0 }; 
    your_addr.addr_to_string(addrstr, 128); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%s) echo: [%d] %s"), addrstr, recv, buf)); 
    udp.send(buf, (size_t)recv, your_addr); 
  }
  else 
    ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("recv")), -1); 

  return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int port = 0; 
  if(argc != 2)
  {
    ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("usage: \n\techo_unicast port\n")), -1); 
  }

  port = atoi(argv[1]); 

  ACE_INET_Addr my_addr(port); 
  //ACE_INET_Addr my_addr(port, "localhost"); 
  //ACE_INET_Addr my_addr; 

  ACE_SOCK_Dgram udp(my_addr); 
  char addrstr[128] = { 0 }; 
  if(udp.get_local_addr(my_addr) == 0 && my_addr.addr_to_string(addrstr, 128) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("local addr = %s.\n"), addrstr)); 

  while(1)
  {
    echo_dgram(udp); 
  }

  udp.close(); 
	return 0;
}

