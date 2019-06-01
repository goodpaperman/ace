// clnt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Log_Msg.h" 
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/SOCK_Connector.h" 
#include "ace/os_ns_errno.h" 
#include "ace/os_ns_netdb.h" 

//#define USE_CONSTRUCTOR
//#define HAS_LOCAL_ADDR
//#define HAS_TIME_OUT
//#define HAS_IO_VEC
//#define HAS_AUTO_IOVEC

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_INET_Addr srvr(50000, ACE_LOCALHOST); 
  //ACE_INET_Addr srvr("telnet", ACE_LOCALHOST); 
  //ACE_INET_Addr srvr("http", "127.0.0.1"); 
  //ACE_INET_Addr srvr("5000", "192.168.0.1"); 
  ACE_SOCK_Stream peer; 

#if defined(USE_CONSTRUCTOR)
  //ACE_OS::last_error(1); 
  ACE_OS::last_error(0); 
  ACE_SOCK_Connector connector(peer, srvr); 
  if(ACE_OS::last_error() != 0)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connector")), -1); 

#else // USE_CONSTRUCTOR
  int retry = 5;  
  ACE_SOCK_Connector connector; 
#  if defined(HAS_LOCAL_ADDR)
  ACE_INET_Addr local(4200, ACE_LOCALHOST); 
#    if defined(WIN32)
  peer.set_handle((ACE_HANDLE)socket(AF_INET, SOCK_STREAM, 0)); 
  int one = 1; 
  one = setsockopt((SOCKET)peer.get_handle(), SOL_SOCKET, SO_REUSEADDR, (char const*)&one, sizeof(one)); 
  if(one < 0)
  {
    ACE_OS::set_errno_to_wsa_last_error(); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("setsockopt"))); 
  }
#    endif // WIN32
  while(connector.connect(peer, srvr, 0, local, 1) == -1 && retry > 0)
#  elif defined(HAS_TIME_OUT)
  ACE_Time_Value timeout(10);
  while(connector.connect(peer, srvr, &timeout) == -1 && retry > 0)
#  else // HAS_TIME_OUT
  while(connector.connect(peer, srvr) == -1 && retry > 0)
#  endif // HAS_LOCAL_ADDR
  {
    //ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connect")), 1); 
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("connect"))); 
    /*if(ACE_OS::last_error() == ETIME)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Timeout while connecting to server\n"))); 
      -- retry; 
      ACE_Time_Value tv(0, 1000); 
      ACE_OS::sleep(tv); 
      continue; 
    }
    else */if(ACE_OS::last_error() == ECONNREFUSED)
    {
      -- retry; 
      ACE_Time_Value tv(0, 1000); 
      ACE_OS::sleep(tv); 
      continue; 
    }
    else 
      return -1; 
  }
#endif /*USE_CONSTRUCTOR*/

  ACE_INET_Addr local; 
  peer.get_local_addr(local); 
  //peer.get_remote_addr(local); 
  
  ACE_TCHAR name[MAXHOSTNAMELEN] = { 0 }; 
  //local.addr_to_string(name, MAXHOSTNAMELEN, 0); 
  local.addr_to_string(name, MAXHOSTNAMELEN); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("connected %s: [OK]\n"), name)); 


  char buf[64] = { 0 }; 
#if defined(HAS_TIME_OUT)
  timeout.set(0, 500); 
  int bc = peer.send_n("uptime\n", 7, &timeout); 
#else 
#  if defined(HAS_IO_VEC)
  iovec send[3] = { 0 }; 
  send[0].iov_base = "uptime\n"; 
  send[0].iov_len = 7; 
  send[1].iov_base = "humidity\n"; 
  send[1].iov_len = 9; 
  send[2].iov_base = "temperature\n"; 
  send[2].iov_len = 13; // include the terminal null char '\0'
  int bc = peer.sendv(send, 3); 
#  else
  int bc = peer.send_n("uptime\n", 7); 
#  endif 
#endif

  if(bc == -1 || bc == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("send_n"))); 
    return -1; 
  }

#if defined(HAS_TIME_OUT)
  timeout.set(1, 100); 
  bc = peer.recv(buf, sizeof(buf), &timeout); 
#else 
#  if defined(HAS_IO_VEC)
#    if defined(HAS_AUTO_IOVEC)
  iovec recvec = { 0 }; 
  bc = peer.recv(&recvec); 
#    else 
  iovec recvec[2] = { 0 }; 
  // an extra char padded for the '\0'
  recvec[0].iov_base = new char[11](); 
  recvec[0].iov_len = 10; 
  recvec[1].iov_base = new char[20](); 
  recvec[1].iov_len = 20; 
  bc = peer.recvv(recvec, 2); 
#    endif 
#  else
  bc = peer.recv(buf, sizeof(buf)); 
#  endif 
#endif 

  if(bc == -1 || bc == 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("recv"))); 
    return -1; 
  }

#if defined(HAS_IO_VEC)
#  if defined(HAS_AUTO_IOVEC)
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] "), bc)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s\n"), recvec.iov_base)); 
  delete [] recvec.iov_base; 
  recvec.iov_base = 0; 
#  else
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] "), bc)); 

  for(int i=0; i<2 && bc > 0; ++ i)
  {
    if(bc < recvec[i].iov_len)
      recvec[i].iov_base[bc] = 0; 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s"), recvec[i].iov_base)); 
    bc -= recvec[i].iov_len; 
    delete [] recvec[i].iov_base; 
    recvec[i].iov_base = 0; 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n"))); 
#  endif 
#else 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("[%d] %s\n"), bc, buf)); 
#endif 

  peer.close(); 
  //srvr.set(60000, ACE_LOCALHOST); 
	return 0;
}

