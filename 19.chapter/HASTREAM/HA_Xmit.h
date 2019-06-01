#pragma once

#include "ace/Module.h" 
#include "ace/SOCK_Stream.h" 
#include "HA_Task.h" 

class HA_Xmit_Out : public HA_Task
{
public:
    virtual int svc (); 
}; 

class HA_Xmit_In : public HA_Task
{
public:
    virtual int init (int argc, ACE_TCHAR *argv[]);
    virtual int fini (); 
    virtual int svc (); 

private:
    bool client_; 
    u_short port_; 
    ACE_TCHAR addrstr_ [MAXHOSTNAMELEN]; 
}; 

class HA_Xmit : public ACE_Module <ACE_MT_SYNCH>/*, public ACE_Service_Object */
{
public:
    HA_Xmit(void);
    ~HA_Xmit(void);

    //virtual int init (int argc, ACE_TCHAR* argv[]); 
    //virtual int fini (); 
    //virtual int info (ACE_TCHAR **info_string, size_t length = 0) const;

private:
    ACE_SOCK_Stream stream_; 
};

