#include "StdAfx.h"
#include "HA_ThreadHook.h"
#include "ace/Log_Msg.h" 

Noop::Noop()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("noop created.\n"))); 
}

Noop::~Noop()
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("noop deleted.\n"))); 
}

ACE_THR_FUNC_RETURN HA_ThreadHook::start(ACE_THR_FUNC func, void* arg)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("thread hook called\n"))); 
   
  tss->this_is = 1; 
  tss->a = 2; 
  tss->no = 3; 
  tss->op = 4; 
  return ACE_Thread_Hook::start(func, arg); 
}