// prodcons.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "prodcons.h" 

int Producer::svc()
{
    int i=0; 
#if defined (USE_SEMA_EXIT)
    for(i=0; i<MAX_PROD + 1; ++ i)
#else 
    for(i=0; i<MAX_PROD + Consumer::N_THREADS; ++ i)
#endif 
        produce_item (i, i >= MAX_PROD); 

    ACE_DEBUG ((LM_DEBUG, "(%t) Producer goes away! i = %u\n", i)); 
    return 0; 
}

void Producer::produce_item (int item, bool hang_up)
{
    //if(this->consumer_.msg_queue ()->deactivated())
    //    return; 

    psema_.acquire(); 
    ACE_Message_Block *mb = new ACE_Message_Block(sizeof(int), 
        hang_up ? ACE_Message_Block::MB_HANGUP : ACE_Message_Block::MB_DATA); 

    ACE_OS::memcpy(mb->wr_ptr (), &item, sizeof (int)); 
    mb->wr_ptr (sizeof (int)); 
    this->consumer_.putq(mb); 

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Produced %d\n"), item)); 
    csema_.release(); 
}


int Consumer::svc ()
{
    while (msg_queue ()->deactivated () == 0)
    {
        csema_.acquire (); 
        ACE_Message_Block *mb = 0; 
        this->getq(mb); 
        if(mb != 0)
        {
            if(mb->msg_type () == ACE_Message_Block::MB_HANGUP)
            {
                shutdown(); 
                mb->release (); 

                ACE_DEBUG ((LM_DEBUG, "(%t) receive hang up message\n")); 
                break; 
            }
            else 
            {
                ACE_DEBUG((LM_DEBUG, 
                    ACE_TEXT("(%t) Consumed %d, num in queue = %u\n"), 
                    *((int *)mb->rd_ptr()), 
                    this->msg_queue ()->message_count ())); 

                mb->release (); 
            }

            psema_.release(); 
        }
        else 
            ACE_DEBUG ((LM_DEBUG, "(%t) message queue deactivated\n")); 
    }

    ACE_DEBUG ((LM_DEBUG, "(%t) exit.\n")); 
    return 0; 
}

void Consumer::shutdown ()
{
#if defined (USE_SEMA_EXIT)
    this->msg_queue ()->deactivate (); 
    csema_.release (N_THREADS); 
#endif 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_Semaphore psem (Consumer::N_THREADS); 
    ACE_Semaphore csem (0); 

    Consumer consumer (psem, csem); 
    Producer producer (psem, csem, consumer); 

    producer.activate (THR_NEW_LWP | THR_JOINABLE, Producer::N_THREADS); 
    consumer.activate (THR_NEW_LWP | THR_JOINABLE, Consumer::N_THREADS); 

    producer.wait (); 
    consumer.wait (); 
	return 0;
}

