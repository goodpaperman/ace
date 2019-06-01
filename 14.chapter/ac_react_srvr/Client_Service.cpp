#include "StdAfx.h"
#include "Client_Service.h"
#include "ace/os_ns_netdb.h" 
#include "ace/os_ns_errno.h" 

Client_Service::Client_Service(void)
// to prevent create the default reactor instance.
: PARENT(0, 0, 0) 
{
}

Client_Service::~Client_Service(void)
{
}

int Client_Service::open(void* p /*= 0*/)
{
  if(PARENT::open(p) == -1)
    return -1; 

  ACE_INET_Addr peer_addr; 
  ACE_TCHAR peer_name[MAXHOSTNAMELEN] = { 0 }; 
  if(peer().get_remote_addr(peer_addr) == 0 && 
    peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN) == 0)
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Connection from %s\n"), peer_name)); 

  return 0; 
}

int Client_Service::handle_input(ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/)
{
  size_t const INPUT_SIZE = 4096; 
  char buffer[INPUT_SIZE] = { 0 }; 
  //ACE_Auto_Array_Ptr<char> ptr(new char[INPUT_SIZE]()); 
  ssize_t recv_cnt(0), send_cnt(0); 

  recv_cnt = this->peer().recv(buffer, INPUT_SIZE-1); 
  if(recv_cnt <= 0)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t) Connection closed.\n")), -1); 
  }

  if(recv_cnt < INPUT_SIZE)
    buffer[recv_cnt] = 0; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) recv message: %s.\n"), buffer)); 
  return 0; 
}

//
//int Client_Service::handle_output(ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/)
//{
//  ACE_Message_Block *mb = 0; 
//  ACE_Time_Value nowait(ACE_OS::gettimeofday()); 
//  while(this->getq(mb, &nowait) == 0)
//  {
//    ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length()); 
//    if(send_cnt == -1)
//      ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("send"))); 
//    else 
//      mb->rd_ptr((size_t)send_cnt); 
//
//    if(mb->length() > 0)
//    {
//      this->ungetq(mb); 
//      break; 
//    }
//
//    mb->release(); 
//  }
//
//  return this->msg_queue()->is_empty() ? -1 : 0; 
//}

int Client_Service::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
  //if(mask == WRITE_MASK)
  //  return 0; 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) handle_close\n"))); 
  return PARENT::handle_close(fd, mask); 
}

Timer::Timer (ACE_Thread_Manager *mgr)
: ACE_Task_Base (mgr)
, timer_ (0)
{
}

Timer::~Timer ()
{
    if(timer_)
    {
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) destructor: cancel timer %u.\n"), timer_)); 
        reactor ()->cancel_timer (timer_); 
        timer_ = 0; 
    }
}

int Timer::svc ()
{
#if 0
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) start reactor event loop !\n"))); 
    //reactor ()->owner (ACE_Thread::self ()); 
    reactor ()->run_reactor_event_loop (); 
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) end reactor event loop !\n"))); 
#else 
    while (thr_mgr ()->testcancel (ACE_Thread::self ()) == 0)
    {
        if(timer_ == 0)
        {
            timer_ = reactor ()->schedule_timer (this, 0, ACE_Time_Value(1, 0)); 
            ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) schedule timer %u.\n", timer_))); 
        }
        else
        {
            reactor ()->cancel_timer (timer_); 
            ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) cancel timer %u.\n", timer_))); 
            timer_ = 0; 
        }

        ACE_OS::sleep (3); 
    }
#endif 

    return 0; 
}


int Timer::handle_timeout (ACE_Time_Value const& time, void const* act)
{
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) handle timeout !\n"))); 
    return 0; 
}


#if defined(ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>; 
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>; 
#endif 