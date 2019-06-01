#pragma once
#include "ace/Acceptor.h"

template <class SVC_HANDLER, ACE_PEER_ACCEPTOR_1>
class ACE_Event_Acceptor : public ACE_Acceptor<SVC_HANDLER, ACE_PEER_ACCEPTOR_2>
{
public:
  /// "Do-nothing" constructor.
  ACE_Event_Acceptor (ACE_Reactor * = 0,
                int use_select = 1);

  /**
   * Initialize and register <this> with the Reactor and listen for
   * connection requests at the designated <local_addr>.  <flags>
   * indicates how <SVC_HANDLER>'s should be initialized prior to
   * being activated.  Right now, the only flag that is processed is
   * <ACE_NONBLOCK>, which enabled non-blocking I/O on the
   * <SVC_HANDLER> when it is opened.  If <use_select> is non-zero
   * then <select> is used to determine when to break out of the
   * <accept> loop.  <reuse_addr> is passed down to the
   * <PEER_ACCEPTOR>.  If it is non-zero this will allow the OS to
   * reuse this listen port.
   */
  ACE_Event_Acceptor (const ACE_PEER_ACCEPTOR_ADDR &local_addr,
                ACE_Reactor * = ACE_Reactor::instance (),
                int flags = 0,
                int use_select = 1,
                int reuse_addr = 1);

  /**
   * Open the contained @c PEER_ACCEPTOR object to begin listening, and
   * register with the specified reactor for accept events.
   *
   * The @c PEER_ACCEPTOR handle is put into non-blocking mode as a
   * safeguard against the race condition that can otherwise occur
   * between the time when the passive-mode socket handle is "ready"
   * and when the actual @c accept call is made.  During this
   * interval, the client can shutdown the connection, in which case,
   * the <accept> call can hang.
   *
   * @param local_addr The address to listen at.
   * @param reactor    Pointer to the ACE_Reactor instance to register
   *                   this object with. The default is the singleton.
   * @param flags      Flags to control what mode an accepted socket
   *                   will be put into after it is accepted. The only
   *                   legal value for this argument is @c ACE_NONBLOCK,
   *                   which enables non-blocking mode on the accepted
   *                   peer stream object in @c SVC_HANDLER.  The default
   *                   is 0.
   * @param use_select Affects behavior when called back by the reactor
   *                   when a connection can be accepted.  If non-zero,
   *                   this object will accept all pending connections,
   *                   intead of just the one that triggered the reactor
   *                   callback.  Uses ACE_OS::select() internally to
   *                   detect any remaining acceptable connections.
   *                   The default is 1.
   * @param reuse_addr Passed to the @c PEER_ACCEPTOR::open() method with
   *                   @p local_addr.  Generally used to request that the
   *                   OS allow reuse of the listen port.  The default is 1.
   *
   * @retval 0  Success
   * @retval -1 Failure, @c errno contains an error code.
   */
  virtual int open (const ACE_PEER_ACCEPTOR_ADDR &local_addr,
                    ACE_Reactor *reactor = ACE_Reactor::instance (),
                    int flags = 0,
                    int use_select = 1,
                    int reuse_addr = 1);

  virtual ~ACE_Event_Acceptor(void);

protected:
  /// Called when object is signaled by OS (either via UNIX signals or
  /// when a Win32 object becomes signaled).
  virtual int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0);
};

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
#include "ACE_Event_Acceptor.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */


#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
#pragma implementation ("ACE_Event_Acceptor.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */