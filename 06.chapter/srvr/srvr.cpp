// srvr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Acceptor.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/Log_Msg.h" 
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 

//#define HAS_TIME_OUT
//#define NO_PEER_ADDR

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  //ACE_INET_Addr port_to_listen(50000, ACE_LOCALHOST); 
  ACE_INET_Addr port_to_listen(50000); 
  ACE_SOCK_Acceptor acceptor; 

  if(acceptor.open(port_to_listen, 1) == -1)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("open acceptor")), -1); 
  }

  ACE_TCHAR peer_name[MAXHOSTNAMELEN] = { 0 }; 
  //acceptor.get_remote_addr(port_to_listen); 
  acceptor.get_local_addr(port_to_listen); 
  port_to_listen.addr_to_string(peer_name, MAXHOSTNAMELEN); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor open %s: [OK]\n"), peer_name)); 

  ACE_SOCK_Stream peer; 
  ACE_INET_Addr peer_addr; 
  while(1)
  {
#if defined(HAS_TIME_OUT)
    ACE_Time_Value timeout(10, 0); 
    if(acceptor.accept(peer, &peer_addr, &timeout, 0) == -1)
#else 
#  if defined(NO_PEER_ADDR)
    if(acceptor.accept(peer) == -1)
#  else
    if(acceptor.accept(peer, &peer_addr) == -1)
#  endif 
#endif 
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("accept"))); 
    }
    else 
    {
#if defined(NO_PEER_ADDR)
      peer.get_remote_addr(peer_addr); 
      //peer.get_local_addr(peer_addr); 
#endif 

      peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection from %s\n"), peer_name)); 
      
      char buffer[4096] = { 0 }; 
      ssize_t bytes_received = 0; 
      //ACE_Time_Value timeout(0, 10000); 
      //while((bytes_received = peer.recv(buffer, sizeof(buffer), 0, &timeout)) != -1)
      while((bytes_received = peer.recv(buffer, sizeof(buffer))) > 0)
      {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv(%d): %s\n"), bytes_received, buffer)); 
        ACE_OS::sleep(1); 
        if(peer.send_n(buffer, bytes_received) == -1)
          break; 
      }

      peer.close(); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection break.\n"))); 
    }
  }

	return 0;
}

