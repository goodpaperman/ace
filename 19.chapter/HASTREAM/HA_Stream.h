#pragma once

#include "ace/Stream.h" 

class HA_Stream : public ACE_Stream <ACE_MT_SYNCH>/*, public ACE_Service_Object*/ 
{
public:
    HA_Stream(void);
    ~HA_Stream(void);

    //virtual int init (int argc, ACE_TCHAR* argv[]); 
    //virtual int fini (); 
    //virtual int info (ACE_TCHAR **info_string, size_t length = 0) const;
};

