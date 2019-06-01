// msgblk.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Message_Block.h" 
#include "ace/Log_Msg.h" 
#include "ace/Timeprobe.h" 

#if defined (ACE_ENABLE_TIMEPROBES)

static const char *ACE_MB_Timeprobe_Description[] =
{
  "Message_Block::clone - enter",
  "Message_Block::clone - leave",
  "Message_Block::duplicate - enter",
  "Message_Block::duplicate - leave", 
  "Message_Block::release - enter", 
  "Message_Block::release - leave", 
};

enum
{
  ACE_CLONE_ENTER = 100, 
  ACE_CLONE_LEAVE, 
  ACE_DUPLICATE_ENTER,
  ACE_DUPLICATE_LEAVE, 
  ACE_RELEASE_ENTER, 
  ACE_RELEASE_LEAVE, 
};


// Setup Timeprobes
ACE_TIMEPROBE_EVENT_DESCRIPTIONS (ACE_MB_Timeprobe_Description,
                                  ACE_CLONE_ENTER);

#endif /* ACE_ENABLE_TIMEPROBES */


void clone_and_release(ACE_Message_Block *mb)
{
  ACE_FUNCTION_TIMEPROBE (ACE_CLONE_ENTER); 
  ACE_Message_Block *nb = mb->clone(); 

  //ACE_TIMEPROBE (ACE_RELEASE_ENTER); 
  nb->release();  
  //ACE_TIMEPROBE (ACE_RELEASE_LEAVE); 
}

void duplicate_and_release(ACE_Message_Block *mb)
{
  ACE_FUNCTION_TIMEPROBE (ACE_DUPLICATE_ENTER); 
  ACE_Message_Block *nb = mb->duplicate(); 

  {
    //ACE_FUNCTION_TIMEPROBE (ACE_RELEASE_ENTER); 
    nb->release(); 
  }
}

void msg_blk_on_stack(ACE_Message_Block *mb)
{
  ACE_Message_Block amb(99), amb2(100), amb3(101); 
  amb.cont(&amb2); 
  amb2.cont(&amb3); 

  //amb.release(); 

  //ACE_Message_Block *pmb = new ACE_Message_Block(98, ACE_Message_Block::MB_DATA, &amb); 
  //pmb->release(); 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10f\n"), 1.23f)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10.3f\n"), 1.23f)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10f\n"), 1.23f)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10.3f\n"), 1.23f)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10d\n"), 123u)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10.5d\n"), 123u)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10d\n"), 123u)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10.5d\n"), 123u)); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10s\n"), "hello")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %10.3s\n"), "hello")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10s\n"), "hello")); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("value = %-10.3s\n"), "hello")); 
  
  printf("\n"); 
  printf("value = %10f\n", 1.23f); 
  printf("value = %10.3f\n", 1.23f); 
  printf("value = %-10f\n", 1.23f); 
  printf("value = %-10.3f\n", 1.23f); 
  printf("value = %10d\n", 123u); 
  printf("value = %10.5d\n", 123u); 
  printf("value = %-10d\n", 123u); 
  printf("value = %-10.5d\n", 123u); 
  printf("value = %10s\n", "hello"); 
  printf("value = %10.3s\n", "hello"); 
  printf("value = %-10s\n", "hello"); 
  printf("value = %-10.3s\n", "hello"); 

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("sizeof empty data block = %d.\n"), sizeof(ACE_Data_Block))); 
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("sizeof empty message block = %d.\n"), sizeof(ACE_Message_Block))); 
  ACE_Message_Block *mb = 0; 
  for(int i=0; i<13; ++ i)
  {
    mb = new ACE_Message_Block(64, ACE_Message_Block::MB_DATA, mb); 
    mb->copy("hello world!\n"); 
  }

  clone_and_release(mb); 
  duplicate_and_release(mb); 
  msg_blk_on_stack(mb); 

  mb->release();   

  ACE_TIMEPROBE_PRINT; 
  //ACE_TIMEPROBE_RESET; 
  ACE_TIMEPROBE_PRINT_ABSOLUTE; 

	return 0;
}

