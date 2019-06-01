#include "StdAfx.h"
#include "ACE_Event_Acceptor.h"

template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1>
ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::ACE_Event_Acceptor (ACE_Reactor *reactor,
                                                              int use_select)
  : ACE_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>(reactor, use_select)
{
  ACE_TRACE ("ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::ACE_Event_Acceptor");
}

template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1>
ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::ACE_Event_Acceptor
  (const ACE_PEER_ACCEPTOR_ADDR &addr,
   ACE_Reactor *reactor,
   int flags,
   int use_select,
   int reuse_addr)
{
  ACE_TRACE ("ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::ACE_Event_Acceptor");

  if (this->open (addr,
                  reactor,
                  flags,
                  use_select,
                  reuse_addr) == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_LIB_TEXT ("%p\n"),
                ACE_LIB_TEXT ("ACE_Event_Acceptor::ACE_Event_Acceptor")));
}



template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1>
ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::~ACE_Event_Acceptor (void)
{
  ACE_TRACE ("ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::~ACE_Event_Acceptor");
}


// Initialize the appropriate strategies for creation, passive
// connection acceptance, and concurrency, and then register <this>
// with the Reactor and listen for connection requests at the
// designated <local_addr>.

template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1> int
ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::open
  (const ACE_PEER_ACCEPTOR_ADDR &local_addr,
   ACE_Reactor *reactor,
   int flags,
   int use_select,
   int reuse_addr)
{
  ACE_TRACE ("ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::open");
  this->flags_ = flags;
  this->use_select_ = use_select;
  this->reuse_addr_ = reuse_addr;
  this->peer_acceptor_addr_ = local_addr;

  // Must supply a valid Reactor to Acceptor::open()...

  if (reactor == 0)
    {
      errno = EINVAL;
      return -1;
    }

  if (this->peer_acceptor_.open (local_addr, reuse_addr) == -1)
    return -1;

  // Set the peer acceptor's handle into non-blocking mode.  This is a
  // safe-guard against the race condition that can otherwise occur
  // between the time when <select> indicates that a passive-mode
  // socket handle is "ready" and when we call <accept>.  During this
  // interval, the client can shutdown the connection, in which case,
  // the <accept> call can hang!
  this->peer_acceptor_.enable (ACE_NONBLOCK);

  // use event as the notify way.
  int result = reactor->register_handler (this, this->get_handle()); 

  if (result != -1)
    this->reactor (reactor);
  else
    this->peer_acceptor_.close ();

  return result;
}

template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1> int
ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::handle_signal (int signum, siginfo_t*, ucontext_t*)
{
  ACE_TRACE ("ACE_Event_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::handle_exception");
  return ACE_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>::handle_input(ACE_INVALID_HANDLE); 
}