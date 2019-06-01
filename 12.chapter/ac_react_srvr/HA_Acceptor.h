#pragma once

#include "ace/Acceptor.h" 
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"  
#include "HA_CommandHandler.h" 
#include "Message_Receiver.h" 

class HA_Acceptor : public ACE_Acceptor<Message_Receiver, ACE_SOCK_ACCEPTOR>
{
public:
  HA_Acceptor(HA_CommandHandler* handler)
    : handler_(handler)
  {
  }

  virtual ~HA_Acceptor(void)
  {
  }

  virtual int make_svc_handler(Message_Receiver *& sh)
  {
    if (sh == 0)
      ACE_NEW_RETURN (sh,
      Message_Receiver(handler_),
      -1);

    // Set the reactor of the newly created <SVC_HANDLER> to the same
    // reactor that this <ACE_Acceptor> is using.
    sh->reactor (this->reactor ());
    return 0;
  }

private:
  HA_CommandHandler* handler_; 
};
