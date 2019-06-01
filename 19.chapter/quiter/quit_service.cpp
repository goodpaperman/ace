#include "StdAfx.h"
#include "quit_service.h"
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/Reactor.h" 
#include "none_service.h" 

#define QUITER_Export _declspec (dllexport)

ACE_FACTORY_DEFINE (QUITER, quit_service)

quit_service::quit_service(void)
{
}

quit_service::~quit_service(void)
{
}


/// Initializes object when dynamic linking occurs.
int quit_service::init (int argc, ACE_TCHAR *argv[])
{
    ACE_DEBUG ((LM_DEBUG, "quit_service::init\n")); 
    NONE_SVC::instance (); 
    //ACE_Reactor::instance ()->end_reactor_event_loop (); 
    return 0; 
}

/// Terminates object when dynamic unlinking occurs.
int quit_service::fini (void)
{
    ACE_DEBUG ((LM_DEBUG, "quit_service::fini\n")); 
    NONE_SVC::close (); 
    return 0; 
}

/// Returns information on a service object.
int quit_service::info (ACE_TCHAR **info_string, size_t length) const
{
    char buf [BUFSIZ] = { 0 }; 
    ACE_OS::sprintf (buf, "-/-   #  service to install quit handler"); 
    ACE_OS::strncpy (*info_string, buf, length); 
    return ACE_OS::strlen (buf); 
}