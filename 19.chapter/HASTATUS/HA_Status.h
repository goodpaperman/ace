#pragma once

#include "ace/Svc_Handler.h" 
#include "ace/INET_Addr.h" 
#include "ace/SOCK_Stream.h" 
#include "ace/SOCK_Acceptor.h" 
#include "ace/Acceptor.h" 
#include "ace/Service_Object.h" 
#include "HASTATUS_export.h" 

class ClientHandler : public ACE_Svc_Handler <ACE_SOCK_STREAM, ACE_NULL_SYNCH> 
{
public:
}; 

class /*HASTATUS_Export*/ HA_Status :  public ACE_Service_Object
{
public:
    HA_Status(void);
    ~HA_Status(void);

    virtual int init (int argc, ACE_TCHAR *argv[]); 
    virtual int fini (void); 
    virtual int info (ACE_TCHAR **str, size_t len) const; 
    virtual int suspend (); 
    virtual int resume (); 

private: 
    ACE_INET_Addr addr_; 
    ACE_Acceptor <ClientHandler, ACE_SOCK_Acceptor> acceptor_; 
};

ACE_FACTORY_DECLARE (HASTATUS, HA_Status) 
