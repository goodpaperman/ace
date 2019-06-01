#include "StdAfx.h"
#include "HA_Acceptor.h"
#include "ace/Log_Msg.h" 

HA_Acceptor::HA_Acceptor(void)
{
}

HA_Acceptor::~HA_Acceptor(void)
{
}

#define BUFSIZE 128 
int HA_Acceptor::validate_connection(
  const ACE_Asynch_Accept::Result& result, 
  const ACE_INET_Addr& remote, 
  const ACE_INET_Addr& local)
{
  struct in_addr* remote_addr = (in_addr*)remote.get_addr(); 
  struct in_addr* local_addr = (in_addr*)local.get_addr(); 
  //return inet_netof(*local_addr) == inet_netof(*remote_addr) ? 0 : -1; 
  char addrstr1[BUFSIZE] = { 0 }, addrstr2[BUFSIZE] = { 0 }; 
  if(remote.addr_to_string(addrstr1, BUFSIZE-1) == 0 && 
    local.addr_to_string(addrstr2, BUFSIZE-1) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("validate_connection: remote %s, local %s\n"), addrstr1, addrstr2)); 

  return remote_addr->S_un.S_un_b.s_b1 == local_addr->S_un.S_un_b.s_b1 ? 0 : -1; 
}
