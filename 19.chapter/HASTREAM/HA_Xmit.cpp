#include "StdAfx.h"
#include "HA_Xmit.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Get_Opt.h" 
#include "ace/OS_NS_errno.h" 

//ACE_FACTORY_DEFINE (ACE_Local_Service, HA_Xmit) 
//ACE_FACTORY_DEFINE (HA_Stream, HA_Xmit) 

extern "C" HA_Stream_Export void* _make_HA_Xmit (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Xmit (); 
}

//ACE_STATIC_SVC_DEFINE (HA_Xmit,
//                       "HA_Xmit", 
//                       ACE_MODULE_T, 
//                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Xmit, 
//                       ACE_Service_Type::DELETE_OBJ |
//                       ACE_Service_Type::DELETE_THIS, 
//                       0) 
//
//ACE_STATIC_SVC_REQUIRE (HA_Xmit) 

HA_Xmit::HA_Xmit(void)
: ACE_Module <ACE_MT_SYNCH> (
                             "HA_Xmit", 
                             new HA_Xmit_In (), 
                             new HA_Xmit_Out (), 
                             (void *)&stream_)
{
}

HA_Xmit::~HA_Xmit(void)
{
}

//int HA_Xmit::init (int argc, ACE_TCHAR* argv[])
//{
//    static const ACE_TCHAR options [] = ACE_TEXT (":cs:p:"); 
//    ACE_Get_Opt opts (argc, argv, options, 0); 
//
//    bool client = false; 
//    u_short port = 8877; 
//    ACE_TCHAR addr_str [MAXHOSTNAMELEN] = { 0 }; 
//    ACE_OS::strcpy (addr_str, "localhost"); 
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
//        case 's':
//            ACE_OS::strncpy (addr_str, opts.opt_arg (), MAXHOSTNAMELEN); 
//            break; 
//        case 'p':
//            port = ACE_OS::atoi (opts.opt_arg ()); 
//            break; 
//        case ':':
//            ACE_ERROR_RETURN ((LM_ERROR, "-%c requires an argument\n", opts.opt_opt ()), -1); 
//        default:
//            ACE_ERROR_RETURN ((LM_ERROR, "Parse error\n"), -1); 
//        }
//    }
//
//    ACE_INET_Addr addr (port, addr_str); 
//    if (client)
//    {
//        ACE_SOCK_Connector conn; 
//        if (conn.connect (stream_, addr) == -1)
//            ACE_ERROR_RETURN ((LM_ERROR, "connect to %s:%u failed.\n", addr_str, port), -1); 
//
//        ACE_DEBUG ((LM_DEBUG, "connect to %s:%u !!!\n", addr_str, port)); 
//    }
//    else 
//    {
//        ACE_SOCK_Acceptor acceptor (addr); 
//        if (acceptor.accept (stream_, &addr) == -1)
//            ACE_ERROR_RETURN ((LM_ERROR, "accept %s:%u failed.\n", addr_str, port), -1); 
//
//        
//        ACE_DEBUG ((LM_DEBUG, "accept from %s:%u !!!\n", addr.get_host_name (), addr.get_port_number ())); 
//    }
//
//    return 0; 
//}
//
//int HA_Xmit::fini ()
//{
//    return 0; 
//}
//
//int HA_Xmit::info (ACE_TCHAR **info_string, size_t length) const
//{
//    return 0; 
//}


int HA_Xmit_Out::svc () 
{
    ACE_DEBUG ((LM_DEBUG, "HA_Xmit_Out startup\n")); 
    ACE_Message_Block *mb = 0; 
    ACE_SOCK_Stream *stream = (ACE_SOCK_Stream *)module ()->arg (); 
    while (getq (mb) == 0)
    {
        //ACE_DEBUG ((LM_DEBUG, "[send] %s", mb->rd_ptr ())); 
        ACE_LOG_MSG->log_hexdump (LM_INFO, mb->rd_ptr (), mb->length (), "[send] "); 
        //ACE_OS::send (mb); 
        stream->send_n (mb); 
        mb->release (); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Xmit_Out shutdown\n")); 
    return 0; 
}

int HA_Xmit_In::init (int argc, ACE_TCHAR *argv[])
{
    static const ACE_TCHAR options [] = ACE_TEXT (":cs:p:"); 
    ACE_Get_Opt opts (argc, argv, options, 0); 

    client_ = false; 
    port_ = 8877; 
    ACE_OS::strcpy (addrstr_, "localhost"); 

    int option = 0; 
    while ((option = opts ()) != EOF)
    {
        switch (option)
        {
        case 'c':
            // to be a client.
            client_ = true; 
            break; 
        case 's':
            ACE_OS::strncpy (addrstr_, opts.opt_arg (), MAXHOSTNAMELEN); 
            break; 
        case 'p':
            port_ = ACE_OS::atoi (opts.opt_arg ()); 
            break; 
        case ':':
            ACE_ERROR_RETURN ((LM_ERROR, "-%c requires an argument\n", opts.opt_opt ()), -1); 
        default:
            ACE_ERROR_RETURN ((LM_ERROR, "Parse error\n"), -1); 
        }
    }

    return 0; 
}

int HA_Xmit_In::fini ()
{
    ACE_DEBUG ((LM_DEBUG, "HA_Xmit fini\n")); 
    ACE_SOCK_Stream *stream = (ACE_SOCK_Stream *)module ()->arg (); 
    stream->close (); 
    return 0; 
}
int HA_Xmit_In::svc () 
{
    int ret = 0; 
    ACE_DEBUG ((LM_DEBUG, "HA_Xmit_In startup\n")); 

    ACE_SOCK_Stream *stream = (ACE_SOCK_Stream *)module ()->arg (); 
    ACE_INET_Addr addr (port_, addrstr_); 

    if (client_)
    {
        ACE_SOCK_Connector conn; 
        if (conn.connect (*stream, addr) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "connect to %s:%u failed.\n", addrstr_, port_), -1); 

        ACE_DEBUG ((LM_DEBUG, "connect to %s:%u !!!\n", addrstr_, port_)); 
    }
    else 
    {
        ACE_SOCK_Acceptor acceptor (addr); 
        if (acceptor.accept (*stream, &addr) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "accept %s:%u failed.\n", addrstr_, port_), -1); 


        ACE_DEBUG ((LM_DEBUG, "accept from %s:%u !!!\n", addr.get_host_name (), addr.get_port_number ())); 
        acceptor.close (); 
    }

    ACE_Time_Value tv (1); 
    ACE_Message_Block *mb = 0; 
    //while (ACE_OS::recv (mb) > 0)
    while (thr_mgr ()->testcancel (ACE_OS::thr_self ()) == 0)
    {
        //ret = ACE_OS::recv (mb); 
        mb = new ACE_Message_Block (4096); 
        ret = stream->recv (mb->rd_ptr (), mb->capacity (), &tv); 
        if (ret <= 0)
        {
            mb->release (); 
            if (ACE_OS::last_error () == ETIME)
                continue; 
            else 
                break; 
        }

        mb->wr_ptr (ret); 
        //ACE_DEBUG ((LM_DEBUG, "[recv] %s", mb->rd_ptr ())); 
        ACE_LOG_MSG->log_hexdump (LM_INFO, mb->rd_ptr (), mb->length (), "[recv] "); 
        this->put_next (mb); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Xmit_In shutdown\n")); 
    return 0; 
}