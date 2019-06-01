#include "StdAfx.h"
#include "HA_Status.h"
#include "ace/Get_Opt.h" 
#include "ace/Configuration.h" 
#include "ace/Configuration_Import_Export.h" 

ACE_FACTORY_DEFINE (ACE_Local_Service, HA_Status) 
//ACE_FACTORY_DEFINE (HASTATUS, HA_Status) 

//ACE_STATIC_SVC_DEFINE (HA_Status_Descriptor, 
//                       "HA_Status_Static_Service", 
//                       ACE_SVC_OBJ_T, 
//                       &ACE_SVC_NAME (HA_Status), 
//                       ACE_Service_Type::DELETE_OBJ | ACE_Service_Type::DELETE_THIS, 
//                       0)
//
//ACE_STATIC_SVC_REQUIRE (HA_Status_Descriptor) 

ACE_STATIC_SVC_DEFINE (HA_Status, 
                       "HA_Status", 
                       ACE_SVC_OBJ_T, 
                       &ACE_SVC_NAME (HA_Status), 
                       ACE_Service_Type::DELETE_OBJ | ACE_Service_Type::DELETE_THIS, 
                       0)

ACE_STATIC_SVC_REQUIRE (HA_Status) 

HA_Status::HA_Status(void)
{
}

HA_Status::~HA_Status(void)
{
}

int HA_Status::init (int argc, ACE_TCHAR *argv[])
{
    static const ACE_TCHAR options [] = ACE_TEXT (":f:"); 
    ACE_Get_Opt opts (argc, argv, options, 0); 
    if (opts.long_option ("config", 'f', ACE_Get_Opt::ARG_REQUIRED) == -1)
        return -1; 

    ACE_TCHAR path [MAXPATHLEN] = { 0 }; 
    ACE_OS::strcpy (path, "HAStatus.conf"); 

    int option = 0; 
    while ((option = opts ()) != EOF)
    {
        switch (option)
        {
        case 'f':
            ACE_OS::strncpy (path, opts.opt_arg (), MAXPATHLEN); 
            break; 
        case ':':
            ACE_ERROR_RETURN ((LM_ERROR, "-%c requires an argument\n", opts.opt_opt ()), -1); 
        default:
            ACE_ERROR_RETURN ((LM_ERROR, "Parse error\n"), -1); 
        }
    }

    ACE_Configuration_Heap config; 
    config.open (); 

    ACE_Registry_ImpExp importer (config); 
    if (importer.import_config (path) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%s: %p\n", "import config", path), -1); 

    ACE_Configuration_Section_Key section; 
    if (config.open_section (config.root_section (), "HAStatus", 1, section) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "open section 'HAStatus'"), -1); 

    u_int port; 
    if (config.get_integer_value (section, "ListenPort", port) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "HAStatus ListenPort does not exist\n"), -1); 

    addr_.set (port); 
    if (acceptor_.open (addr_) != 0)
        ACE_ERROR_RETURN ((LM_ERROR, "HAStatus %p\n", "accept"), -1); 

    ACE_DEBUG ((LM_INFO, "service HA_Status startup at port %d!!!\n", port)); 
    return 0; 
}

int HA_Status::fini (void)
{
    acceptor_.close (); 
    return 0; 
}

int HA_Status::info (ACE_TCHAR **str, size_t len) const
{
    if (str == 0)
        return -1; 

    if (*str == 0)
    {
        len = 128; 
        ACE_NEW_RETURN (*str, ACE_TCHAR[len] (), -1); 
    }

    return ACE_OS::snprintf (*str, len, "%d/tcp    # home automation status server\n", addr_.get_port_number ()); 
}