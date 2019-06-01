// activeobj.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <memory> 
#include "ace/OS_NS_unistd.h" 
#include "ace/Task.h" 
#include "ace/Future.h" 
#include "ace/Method_Request.h" 
#include "ace/Activation_Queue.h" 
#include "ace/Log_Msg.h" 

#define USE_TASKQ
//#define USE_CALLBACK
#define NUM_THRS 2

class HA_ControllerAgent
{
public:
    HA_ControllerAgent ()
    {
        status_result_ = 1; 
    }

    int status_update ()
    {
        ACE_OS::sleep (2); 
        int ret =  next_result_id (); 
        ACE_DEBUG ((LM_DEBUG, "(%t) Obtaining a status_update of %d\n", ret)); 
        return ret; 
    }

private:
    int next_result_id () { return status_result_ ++; } 

    int status_result_; 
}; 

#if defined (USE_TASKQ)
class Scheduler : public ACE_Task <ACE_MT_SYNCH>
#else 
class Scheduler : public ACE_Task_Base 
#endif 
{
public:
    Scheduler ()
#if defined (USE_TASKQ)
        : activation_queue_ (msg_queue ())
#endif 
    {
        this->activate (THR_NEW_LWP | THR_JOINABLE, NUM_THRS); 
    }

    virtual int svc () 
    {
        while (1)
        {
            std::auto_ptr <ACE_Method_Request> request (this->activation_queue_.dequeue ()); 
            if(request->call () == -1)
                break; 
        }

        return 0; 
    }

    int enqueue (ACE_Method_Request *request)
    {
        return activation_queue_.enqueue (request); 
    }

private:
    ACE_Activation_Queue activation_queue_; 
}; 

class StatusUpdate : public ACE_Method_Request
{
public:
    StatusUpdate (HA_ControllerAgent &controller, ACE_Future <int> &ret)
        : controller_ (controller)
        , ret_ (ret) 
    {
    }

    virtual int call ()
    {
        ret_.set (controller_.status_update ()); 
        return 0; 
    }

private: 
    HA_ControllerAgent &controller_; 
    ACE_Future <int> &ret_; 
}; 

class ExitMethod : public ACE_Method_Request
{
public:
    virtual int call ()
    {
        return -1; 
    }
}; 


class HA_ControllerAgentProxy
{
public:
    void status_update (ACE_Future <int> &result)
    {
        scheduler_.enqueue (new StatusUpdate (controller_, result)); 
    }

    void exit ()
    {
        scheduler_.enqueue (new ExitMethod ()); 
    }

private:
    Scheduler scheduler_; 
    HA_ControllerAgent controller_; 
}; 


#if defined (USE_CALLBACK)
class CompletionCallBack : public ACE_Future_Observer <int> 
{
public:
    CompletionCallBack (HA_ControllerAgentProxy &proxy)
        : proxy_ (proxy)
    {
    }

    virtual void update (ACE_Future<int> const& future)
    {
        int result = 0; 
        ((ACE_Future <int> &) future).get (result); 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT("(%t) New Status %d\n"), result)); 
        if (result == 10)
        {
            for (int k=0; k<NUM_THRS; ++ k)
                proxy_.exit (); 
        }
    }

private:
    HA_ControllerAgentProxy &proxy_; 
}; 
#endif 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    HA_ControllerAgentProxy controller; 
    ACE_Future <int> results[10]; 
#if defined (USE_CALLBACK)
    CompletionCallBack cb (controller); 
#endif 

    for (int i=0; i<10; ++ i)
    {
        controller.status_update (results[i]); 
#if defined (USE_CALLBACK)
        results[i].attach (&cb); 
#endif 
    }

    ACE_OS::sleep (10); 

#if !defined (USE_CALLBACK)
    int result; 
    for (int j=0; j<10; ++ j)
    {
        results[j].get (result); 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) New status_update %d\n"), result)); 
    }

    for (int k=0; k<NUM_THRS; ++ k)
        controller.exit (); 
#endif 

    ACE_Thread_Manager::instance ()->wait (); 
	return 0;
}

