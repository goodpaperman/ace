#include "StdAfx.h"
#include "HA_Server.h"

extern "C" HA_Stream_Export void* _make_HA_Server (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Server (); 
}

//ACE_STATIC_SVC_DEFINE (HA_Server,
//                       "HA_Server", 
//                       ACE_MODULE_T, 
//                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Server, 
//                       ACE_Service_Type::DELETE_OBJ |
//                       ACE_Service_Type::DELETE_THIS, 
//                       0) 
//
//ACE_STATIC_SVC_REQUIRE (HA_Server) 

HA_Server::HA_Server(void)
: ACE_Module <ACE_MT_SYNCH> (
                             "HA_Server", 
                             new HA_Server_Convert (), 
                             new ACE_Thru_Task <ACE_MT_SYNCH> ())
{
}

HA_Server::~HA_Server(void)
{
}
