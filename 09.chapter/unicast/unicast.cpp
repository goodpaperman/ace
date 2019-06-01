// unicast.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <algorithm> 

#include "ace/Log_Msg.h" 
#include "ace/INET_Addr.h" 
#include "ace/Signal.h" 

// #define CONNECTIONLESS

#if defined(CONNECTIONLESS)
#include "ace/SOCK_Dgram.h" 
#else
#include "ace/SOCK_CODgram.h" 
#endif 

int send_unicast(
#if defined(CONNECTIONLESS)
                 ACE_SOCK_Dgram &udp, ACE_INET_Addr const& to
#else 
                 ACE_SOCK_CODgram &udp
#endif 
                 )
{
  char const* message = "this is the message!\n"; 
  ssize_t sent = udp.send(message, strlen(message)+1
#if defined(CONNECTIONLESS)
    , to
#endif 
    ); 

  if(sent == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("send")), -1); 
  else 
  {
    char addrstr[128] = { 0 }; 
#if !defined(CONNECTIONLESS) 
    ACE_INET_Addr to; 
    udp.get_remote_addr(to); 
#endif 

    to.addr_to_string(addrstr, 128); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%s) send: %d\n"), addrstr, sent)); 
  }

  return 0; 
}

int recv_unicast(
#if defined(CONNECTIONLESS)
                 ACE_SOCK_Dgram &udp, ACE_INET_Addr &from
#else 
                 ACE_SOCK_CODgram &udp
#endif 
                 )
{
  char buf[1024] = { 0 }; 
  int buflen = 1024; 
  ssize_t recv = udp.recv(buf, buflen
#if defined(CONNECTIONLESS)
    , from
#endif 
    ); 

  if(recv == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("recv")), -1); 
  else 
  {
    char addrstr[128] = { 0 }; 
#if !defined(CONNECTIONLESS) 
    ACE_INET_Addr from; 
    udp.get_remote_addr(from); 
#endif 

    from.addr_to_string(addrstr, 128); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%s) recv: [%d] %s"), addrstr, recv, buf)); 
  }

  return 0; 
}


int _running = 1; 
void handle_signal(int signum)
{
  ACE_DEBUG((LM_DEBUG, "Ctrl+C pressed by user, exiting.\n")); 
  _running = 0; 
}


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int min_port = 0; 
  int max_port = 0; 
  if(argc != 3)
  {
    ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("usage: \n\tunicast minport maxport\n")), -1); 
  }

  min_port = atoi(argv[1]); 
  max_port = atoi(argv[2]); 
  if(min_port > max_port)
    std::swap(min_port, max_port); 

  char addrstr[128] = { 0 }; 

#if defined(CONNECTIONLESS)
  //ACE_INET_Addr my_addr(10101); 
  ACE_INET_Addr my_addr; 

  //ACE_SOCK_Dgram udp(my_addr); 
  ACE_SOCK_Dgram udp(ACE_Addr::sap_any); 
#else 
  ACE_INET_Addr local_addr; 
  //ACE_INET_Addr local_addr(7788); 
  //ACE_INET_Addr local_addr(7788, "localhost"); 
  //ACE_Addr local_addr = ACE_Addr::sap_any; 
  
  ACE_INET_Addr my_addr(min_port, "localhost"); 

  //ACE_SOCK_CODgram udp(ACE_Addr::sap_any, local_addr); 
  //ACE_SOCK_CODgram udp(ACE_Addr::sap_any); 
  ACE_SOCK_CODgram udp(my_addr, local_addr); 
#endif 
  
  //ACE_INET_Addr local(10103, ACE_LOCALHOST); //"192.168.0.1"); 
  //if (ACE_OS::bind (ACE_OS::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP), 
  //  ACE_reinterpret_cast (sockaddr *,
  //  local.get_addr ()),
  //  local.get_size ()) == -1)
  //{
  //  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p"), ACE_TEXT("bind"))); 
  //  return -1; 
  //}

  if(udp.get_local_addr(my_addr) == 0 && my_addr.addr_to_string(addrstr, 128) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("local addr = %s.\n"), addrstr)); 

#if defined(CONNECTIONLESS)
  int port = min_port; 
  ACE_INET_Addr to((u_short)0, "127.0.0.1"); 
#endif 

  ACE_Sig_Action sigact(handle_signal, SIGINT); 
  while(_running)
  {
#if defined(CONNECTIONLESS)
    to.set_port_number(port++); 
    if(port > max_port)
      port = min_port; 
#endif 

    send_unicast(udp
#if defined(CONNECTIONLESS)
      , to
#endif 
      ); 

    recv_unicast(udp
#if defined(CONNECTIONLESS)
      , to
#endif 
      ); 
  }

  udp.close(); 
	return 0;
}

