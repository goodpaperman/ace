// tss.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/TSS_T.h" 
#include "ace/Task.h" 
#include <map> 
using std::map; 

class ClientContext
{
public:
    ClientContext ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) context created\n")); 
    }

    ~ClientContext ()
    {
        ACE_DEBUG ((LM_DEBUG, "(%t) context destroyed\n")); 
    }

    void* get_attribute (char const* name)
    {
        map <char const*, void*>::iterator it = map_.find (name); 
        return it == map_.end () ? 0 : it->second; 
    }

    void set_attribute (char const* name, void *value)
    {
        map_.insert (std::make_pair (name, value)); 
    }

private:
    map <char const*, void*> map_; 
}; 

class HA_CommandHandler : public ACE_Task <ACE_MT_SYNCH> 
{
public:
    virtual int svc ()
    {
        ACE_thread_t tid = ACE_Thread::self (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) is running.\n")); 
        tss_ctx_->set_attribute ("thread_id", &tid); 

        ACE_thread_t *tid2 = (ACE_thread_t *)tss_ctx_->get_attribute ("thread_id"); 
        if (tid2) 
            ACE_DEBUG ((LM_DEBUG, "(%t) TSS tid: %u\n", *tid2)); 

        ACE_Thread::exit (tid); 
        return 0; 
    }

private:
    ACE_TSS <ClientContext> tss_ctx_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    HA_CommandHandler handler; 
    handler.activate (THR_NEW_LWP | THR_JOINABLE, 5); 
    handler.wait (); 
	return 0;
}

