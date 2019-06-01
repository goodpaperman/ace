#include "StdAfx.h"
#include "HA_CommandHandler.h"

HA_CommandHandler::HA_CommandHandler(HA_Device_Repository &rep)
: rep_(rep)
{
}

HA_CommandHandler::~HA_CommandHandler(void)
{
}

int HA_CommandHandler::svc()
{
    for(int i=0; i<HA_Device_Repository::N_DEVICES; ++ i)
        rep_.update_device(i, ""); 

    return 0; 
}
