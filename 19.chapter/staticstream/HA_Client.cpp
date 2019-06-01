#include "StdAfx.h"
#include "HA_Client.h"

extern "C" HA_Stream_Export void* _make_HA_Client (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Client (); 
}

ACE_STATIC_SVC_DEFINE (HA_Client,
                       "HA_Client", 
                       ACE_MODULE_T, 
                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Client, 
                       ACE_Service_Type::DELETE_OBJ |
                       ACE_Service_Type::DELETE_THIS, 
                       0) 

ACE_STATIC_SVC_REQUIRE (HA_Client) 

HA_Client::HA_Client(void)
: ACE_Module <ACE_MT_SYNCH> (
                             "HA_Client", 
                             new HA_Client_Driver (), 
                             new ACE_Thru_Task <ACE_MT_SYNCH> ())
{
}

HA_Client::~HA_Client(void)
{
}
