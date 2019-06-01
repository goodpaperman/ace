#pragma once

#include "ace/Service_Object.h" 

class quit_service : public ACE_Service_Object
{
public:
    quit_service(void);
    virtual ~quit_service(void);

    /// Initializes object when dynamic linking occurs.
    virtual int init (int argc, ACE_TCHAR *argv[]);

    /// Terminates object when dynamic unlinking occurs.
    virtual int fini (void);

    /// Returns information on a service object.
    virtual int info (ACE_TCHAR **info_string, size_t length = 0) const;
};
