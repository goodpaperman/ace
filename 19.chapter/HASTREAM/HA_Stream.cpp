#include "StdAfx.h"
#include "ace/Service_Config.h" 
#include "ace/Get_Opt.h" 
#include "HA_Stream.h"
#include "HA_Server.h"
#include "HA_Client.h" 

//ACE_FACTORY_DEFINE (ACE_Local_Service, HA_Stream) 


extern "C" HA_Stream_Export void* _make_HA_Stream (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Stream (); 
}

//ACE_STATIC_SVC_DEFINE (HA_Stream,
//                       "HA_Stream", 
//                       ACE_STREAM_T, 
//                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Stream, 
//                       ACE_Service_Type::DELETE_OBJ |
//                       ACE_Service_Type::DELETE_THIS, 
//                       0) 
//
//ACE_STATIC_SVC_REQUIRE (HA_Stream) 

HA_Stream::HA_Stream(void)
{
}

HA_Stream::~HA_Stream(void)
{
}


//int HA_Stream::init (int argc, ACE_TCHAR* argv[])
//{
//    static const ACE_TCHAR options [] = ACE_TEXT (":c"); 
//    ACE_Get_Opt opts (argc, argv, options, 0); 
//
//    int ret = 0; 
//    bool client = false; 
//
//    int option = 0; 
//    while ((option = opts ()) != EOF)
//    {
//        switch (option)
//        {
//        case 'c':
//            // to be a client.
//            client = true; 
//            break; 
//        case ':':
//            ACE_ERROR_RETURN ((LM_ERROR, "-%c requires an argument\n", opts.opt_opt ()), -1); 
//        default:
//            ACE_ERROR_RETURN ((LM_ERROR, "Parse error\n"), -1); 
//        }
//    }
//
//    if (client)
//    {
//        ACE_DEBUG ((LM_DEBUG, "push client module\n"));
//        ret = this->push (new HA_Client ()); 
//    }
//    else 
//    {
//        ACE_DEBUG ((LM_DEBUG, "push server module\n"));
//        ret = this->push (new HA_Server ()); 
//    }
//
//    return ret; 
//}
//
//int HA_Stream::fini ()
//{
//    return 0; 
//}
//
//int HA_Stream::info (ACE_TCHAR **info_string, size_t length) const
//{
//    return 0; 
//}