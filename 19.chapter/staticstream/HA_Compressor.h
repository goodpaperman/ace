#pragma once

#include "ace/Module.h" 
#include "HA_Task.h" 
#include "zlib.h"

class HA_Compressor_Out : public HA_Task
{
public:
    virtual int svc (); 
    virtual int fini (); 
    int compress (ACE_Message_Block *mb, ACE_Message_Block *&mb_out); 

protected:
    z_stream zstream_;
}; 

class HA_Compressor_In : public HA_Task 
{
public:
    virtual int svc (); 
    int uncompress (ACE_Message_Block *mb, ACE_Message_Block *&mb_out); 

protected:
    z_stream zstream_;
}; 

class HA_Compressor : public ACE_Module <ACE_MT_SYNCH>/* , public ACE_Service_Object */
{
public:
    HA_Compressor(void); 
    ~HA_Compressor(void); 
};

//ACE_FACTORY_DECLARE (ACE_Local_Service, HA_Compressor)
//ACE_FACTORY_DECLARE (HA_Stream, HA_Compressor)