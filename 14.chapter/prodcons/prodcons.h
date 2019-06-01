
#pragma once
#include "ace/Task.h" 
#include "ace/Semaphore.h" 

//#define USE_SEMA_EXIT

class Consumer; 
class Producer : public ACE_Task_Base
{
public:
    enum { 
        MAX_PROD = 1280, 
        N_THREADS = 1, // should be single.
    }; 

    Producer (ACE_Semaphore &psema, ACE_Semaphore &csema, Consumer &consumer)
        : psema_(psema), csema_(csema), consumer_(consumer)
    {
    }

    virtual int svc (); 
    void produce_item (int item, bool hang_up = false); 

private:
    ACE_Semaphore &psema_; 
    ACE_Semaphore &csema_; 
    Consumer &consumer_; 
}; 

class Consumer : public ACE_Task<ACE_MT_SYNCH>
{
public:
    enum { 
        N_THREADS = 5, 
    }; 

    Consumer (ACE_Semaphore &psema, ACE_Semaphore &csema)
        : psema_(psema), csema_(csema)
    {
    }

    virtual int svc (); 
    void shutdown (); 

private:
    ACE_Semaphore &psema_; 
    ACE_Semaphore &csema_; 
}; 