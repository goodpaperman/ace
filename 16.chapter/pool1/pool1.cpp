// pool1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Condition_Thread_Mutex.h" 
#include "ace/OS_NS_time.h" 
#include "ace/OS_NS_unistd.h" 


class Worker; 
class IManager 
{
public:
    virtual int return_to_work (Worker *worker) = 0; 
}; 


class Worker : public ACE_Task <ACE_MT_SYNCH>
{
public:
    enum { 
        WORK_TIME = 100, 
    }; 

    Worker (IManager *manager)
        : manager_ (manager)
        , thread_id_ (0) 
    {
    }

    virtual int svc ()
    {
        thread_id_ = ACE_Thread::self (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) worker started.\n")); 

        while (1)
        {
            ACE_Message_Block *mb = 0; 
            ACE_ASSERT (getq (mb) != -1); 
            if(mb->msg_type () == ACE_Message_Block::MB_HANGUP)
            {
                mb->release (); 
                ACE_DEBUG ((LM_DEBUG, "(%t) shutting down\n")); 
                break; 
            }

            process_message (mb); 
            manager_->return_to_work (this); 
        }

        return 0; 
    }

    void process_message (ACE_Message_Block *mb)
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) process message: %s.\n", mb->base())); 
        mb->release (); 
        ACE_OS::sleep (ACE_Time_Value (0, WORK_TIME)); 
    }

    ACE_thread_t thr_id () const { return thread_id_; } 

private:
    ACE_thread_t thread_id_; 
    IManager *manager_; 
}; 



class Manager : public ACE_Task <ACE_MT_SYNCH>, IManager
{
public:
    enum 
    {
        POOL_SIZE = 5, 
        MAX_TIMEOUT = 100, 
    }; 

    Manager ()
        : shutdown_ (0)
        , workers_lock_ ()
        , workers_cond_ (workers_lock_)
    {
    }

    virtual int svc ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) Manager started\n")); 
        create_worker_pool (); 

        while (!done ())
        {
            ACE_Time_Value tv (0, (long) MAX_TIMEOUT); 
            tv += ACE_OS::time (0); 

            ACE_Message_Block *mb = 0; 
            if (getq (mb, &tv) == -1)
            {
                shutdown (); 
                break; 
            }

            Worker *worker = 0; 

            {
                ACE_GUARD_RETURN (ACE_Thread_Mutex, worker_mon, this->workers_lock_, -1); 
                while (this->workers_.is_empty ())
                    workers_cond_.wait (); 

                this->workers_.dequeue_head (worker); 
            }

            worker->putq (mb); 
        }

        return 0; 
    }

    int shutdown ()
    {
        ACE_GUARD_RETURN (ACE_Thread_Mutex, worker_mon, workers_lock_, -1); 
        ACE_DEBUG ((LM_DEBUG, "prepare to shutdown\n")); 
        shutdown_ = 1; 

        // notify the worker to exit.
        Worker **worker = 0; 
        for (ACE_Unbounded_Queue <Worker *>::ITERATOR it (workers_); 
            it.next (worker) != 0; it.advance ())
        {
            (*worker)->putq (new ACE_Message_Block (0, ACE_Message_Block::MB_HANGUP)); 
        }

        return 0; 
    }

    virtual int return_to_work (Worker *worker)
    {
        ACE_GUARD_RETURN (ACE_Thread_Mutex, worker_mon, workers_lock_, -1); 
        //ACE_DEBUG ((LM_DEBUG, "(%t) Worker %d returning to work.\n", worker->thr_mgr ()->thr_self ())); 
        ACE_DEBUG ((LM_DEBUG, "(%t) Worker %d returning to work.\n", worker->thr_id ())); 
        workers_.enqueue_tail (worker); 
        workers_cond_.signal (); 

        if (shutdown_)
            worker->putq (new ACE_Message_Block (0, ACE_Message_Block::MB_HANGUP)); 

        return 0; 
    }

    int create_worker_pool ()
    {
        ACE_GUARD_RETURN (ACE_Thread_Mutex, worker_mon, workers_lock_, -1); 
        for (int i = 0; i<POOL_SIZE; ++ i)
        {
            Worker *worker = 0; 
            ACE_NEW_RETURN (worker, Worker (this), -1); 
            this->workers_.enqueue_tail (worker); 
            worker->activate (); 
        }

        return 0; 
    }

    int done ()
    {
        return shutdown_; 
    }

private:
    int shutdown_; 
    ACE_Thread_Mutex workers_lock_; 
    //ACE_Condition <ACE_Thread_Mutex> workers_cond_; 
    ACE_Condition_Thread_Mutex workers_cond_; 
    ACE_Unbounded_Queue <Worker *> workers_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    Manager mgr; 
    mgr.activate (); 

    ACE_Message_Block *mb = 0; 
    for (int i=0; i<10000; ++ i)
    {
        ACE_NEW_RETURN (mb, ACE_Message_Block (16), -1); 
        mb->wr_ptr (ACE_OS::sprintf (mb->base (), "%d", i+1)); 
        mgr.putq (mb); 
    }

    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

