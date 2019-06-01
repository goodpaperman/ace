// lf.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/OS_NS_unistd.h" 

#define ON_STACK
//#define FOLLOWER_LOCK  // I think this lock is redundant 

class Follower_Cond : public ACE_Condition_Thread_Mutex
{
public:
    Follower_Cond (ACE_Thread_Mutex &mutex) 
        : ACE_Condition_Thread_Mutex (mutex)
        , owner_ (ACE_Thread::self ())
    {
    }

    ACE_thread_t owner () const { return owner_; } 

private:
    ACE_thread_t owner_; 
}; 

class LF_ThreadPool : public ACE_Task <ACE_MT_SYNCH> 
{
public:
    enum { 
        MAX_WAITTIME = 200, 
    }; 

    LF_ThreadPool () 
        : shutdown_ (0)
        , current_leader_ (0) 
    {
        //msg_queue ()->open (1024*1024, 1024*1024); 
    }

    void shutdown () { shutdown_ = 1; } 
    int done () { return shutdown_; } 
    int leader_active () { return current_leader_ != 0; } 
    void leader_active (ACE_thread_t leader) { current_leader_ = leader; } 
    void process_message (ACE_Message_Block *mb)
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) process message: %s\n", mb->base ())); 
        mb->release (); 
        ACE_OS::sleep (ACE_Time_Value (0, 10)); 
    }

    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) start up\n")); 
        while (!done ())
        {
            become_leader (); 

            ACE_Message_Block *mb = 0; 
            ACE_Time_Value tv (0, MAX_WAITTIME); 
            tv += ACE_OS::gettimeofday (); 

            if(getq (mb, &tv) == -1)
            {
                // notify the follower to exit too.
                elect_new_leader (); 

                break; 
            }

            elect_new_leader (); 
            process_message (mb); 
        }

        ACE_DEBUG ((LM_DEBUG, "(%t) exit\n")); 
        return 0; 
    }


    int become_leader ()
    {
        ACE_GUARD_RETURN (ACE_Thread_Mutex, mon, leader_lock_, -1); 
        if (leader_active ())
        {
#if defined (ON_STACK)
            Follower_Cond cond_s (leader_lock_), *cond (&cond_s); 
#else 
            Follower_Cond *cond = 0; 
            ACE_NEW_RETURN (cond, Follower_Cond (leader_lock_), -1); 
#endif 

#if 0
            {
#if defined (FOLLOWER_LOCK)
                ACE_GUARD_RETURN (ACE_Thread_Mutex, guard, followers_lock_, -1); 
#endif 
                followers_.enqueue_tail (cond); 
            }

            while (leader_active ())
                cond->wait (); 
#else 
            int n = 0; 
            while (leader_active ())
            {
                ACE_DEBUG ((LM_DEBUG, "(%t) waiting to become leader, %u times\n", ++n)); 
                // NOTE: this re-enqueue very important, 
                // as when we signalled by other thread, 
                // we have dequeued, if someone else
                // become leader during the gap, 
                // and we go to wait again without enqueue, 
                // we will never get notified.
                followers_.enqueue_tail (cond); 
                cond->wait (); 
                ACE_DEBUG ((LM_DEBUG, "(%t) get signalled\n")); 
            }
#endif 

#if !defined (ON_STACK)
            delete cond; 
#endif 
        }

        ACE_DEBUG ((LM_DEBUG, "(%t) Becoming the leader.\n")); 
        leader_active (ACE_Thread::self ()); 
        return 0; 
    }

    int elect_new_leader ()
    {
        ACE_GUARD_RETURN (ACE_Thread_Mutex, mon, leader_lock_, -1); 
        leader_active (0); 
        if(!followers_.is_empty ())
        {
            Follower_Cond *cond = 0; 
#if defined (FOLLOWER_LOCK)
            ACE_GUARD_RETURN (ACE_Thread_Mutex, guard, followers_lock_, -1); 
#endif 
            int ret = followers_.dequeue_head (cond); 
            ACE_ASSERT (ret == 0); 
            ACE_DEBUG ((LM_DEBUG, "(%t) Resigning and Electing %d\n", cond->owner ())); 
            cond->signal (); 
            return 0; 
        }
        else 
            ACE_DEBUG ((LM_DEBUG, "(%t) Oops no followers left, size = %u\n", followers_.size ())); 
        
        return -1; 
    }

private:
    int shutdown_; 
    ACE_thread_t current_leader_; 
    ACE_Thread_Mutex leader_lock_; 
    ACE_Unbounded_Queue <Follower_Cond *> followers_; 
#if defined (FOLLOWER_LOCK)
    ACE_Thread_Mutex followers_lock_; 
#endif 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    LF_ThreadPool pool; 
    ACE_Message_Block *mb = 0; 
    for (int i=0; i<10; ++ i)
    {
        ACE_NEW_RETURN (mb, ACE_Message_Block (64), -1); 
        ACE_OS::sprintf (mb->base (), "%d", i+1); 
        mb->wr_ptr (ACE_OS::strlen (mb->base ())); 
        pool.putq (mb); 
    }

    pool.activate (THR_NEW_LWP | THR_JOINABLE, 3); 
    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

