#include "StdAfx.h"
#include "acceptor_task.h"
#include "react_acceptor.h" 

acceptor_task::acceptor_task(void)
{
}

acceptor_task::~acceptor_task(void)
{
}

int acceptor_task::svc()
{
  // wait the main thread run event loop first.
  ACE_OS::sleep(3); 

  react_acceptor *acceptor = 0; 
  ACE_INET_Addr port_to_listen(50000); 
  ACE_NEW_RETURN(acceptor, react_acceptor(), -1); 

  acceptor->reactor(this->reactor()); 
  if(acceptor->open(port_to_listen) == -1)
    return -1; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("acceptor opened.\n"))); 
  while(ACE_Thread_Manager::instance()->testcancel(ACE_Thread::self()) == 0)
  {
    ACE_OS::sleep(1); 
  }

  // no need to delete acceptor, 
  // it is registered in reactor, 
  // and reactor will response for 
  // clean it up.
  //delete acceptor; 
  return 0; 
}
