// prio_clnt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/SOCK_Connector.h" 
#include "ace/os_ns_errno.h" 
#include "ace/os_ns_netdb.h" 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_INET_Addr srvr(50000, ACE_LOCALHOST); 
  //ACE_INET_Addr srvr("telnet", ACE_LOCALHOST); 
  //ACE_INET_Addr srvr("http", "127.0.0.1"); 
  //ACE_INET_Addr srvr("5000", "192.168.0.1"); 
  ACE_SOCK_Stream peer[10]; 

  int retry = 5;  
  ACE_SOCK_Connector connector; 
  for(int i=0; i<10; ++ i)
  {
    while(connector.connect(peer[i], srvr) == -1)
    {
      if(retry == 0)
        return -1; 

      //ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connect")), 1); 
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("connect"))); 
      if(ACE_OS::last_error() == ECONNREFUSED)
      {
        -- retry; 
        ACE_Time_Value tv(0, 1000); 
        ACE_OS::sleep(tv); 
        continue; 
      }
      else 
        return -1; 
    }
  }

  ACE_OS::sleep(1); 
  for(int i=0; i<10; ++ i)
  {
    ACE_INET_Addr local; 
    peer[i].get_local_addr(local); 
    //peer.get_remote_addr(local); 

    ACE_TCHAR name[MAXHOSTNAMELEN] = { 0 }; 
    //local.addr_to_string(name, MAXHOSTNAMELEN, 0); 
    local.addr_to_string(name, MAXHOSTNAMELEN); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("connected %s: [OK]\n"), name)); 

    int bc = peer[i].send_n("uptime ", 7); 
    if(bc == -1 || bc == 0)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("send_n"))); 
      return -1; 
    }
  }

  ACE_OS::sleep(1); 
  for(int i=0; i<10; ++ i)
  {
    char buf[64] = { 0 }; 
    int bc = peer[i].recv(buf, sizeof(buf)-1); 
    if(bc == -1 || bc == 0)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("recv"))); 
      return -1; 
    }
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] %s\n"), bc, buf)); 
  }

  ACE_OS::sleep(1); 
  for(int i=0; i<10; ++ i)
  {
    peer[i].close(); 
    //srvr.set(60000, ACE_LOCALHOST); 
  }

	return 0;
}


