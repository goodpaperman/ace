// event.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Task.h" 
#include "ace/Auto_Event.h" 
#include "ace/Manual_Event.h" 
#include "ace/OS_NS_unistd.h" 

//#define DELAYED_WAIT
//#define USE_AUTO
#define USE_SINGAL
//#define USE_PULSE
//#define RESET_EVENT

#if defined (USE_AUTO)
typedef ACE_Auto_Event EVENT_TYPE; 
#else 
typedef ACE_Manual_Event EVENT_TYPE; 
#endif 

class workhorse : public ACE_Task_Base
{
public:
    workhorse (EVENT_TYPE &e)
        : e_ (e)
    {
    }

    virtual int svc ()
    {
#if defined (DELAYED_WAIT)
        ACE_OS::sleep (2); 
#endif 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) work horse start.\n"))); 
        e_.wait (); 
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) wait event.\n"))); 
        return 0; 
    }

private:
    EVENT_TYPE &e_; 
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    EVENT_TYPE e (0); 
    workhorse wh (e); 
    wh.activate (THR_NEW_LWP | THR_JOINABLE, 2); 

    ACE_OS::sleep (1); 
#if defined (USE_SINGAL)
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) signal event.\n"))); 
    e.signal (); 
#elif defined (USE_PULSE)
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) pulse event.\n"))); 
    e.pulse (); 
#endif 

#if defined (RESET_EVENT)
    ACE_OS::sleep(1); 
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) reset event.\n"))); 
    e.reset (); 
#endif 

    ACE_OS::sleep (1); 
    ACE_Thread_Manager::instance ()->cancel_task (&wh); 
    wh.wait (); 
	return 0;
}

