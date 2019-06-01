// argv.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/ARGV.h" 
#include "ace/Get_Opt.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  static const ACE_TCHAR options[] = ACE_TEXT(":f:h:"); 
  static const ACE_TCHAR cmdline[] = ACE_TEXT("-f $OS/home/managed.cfg -h $OS"); 
  ACE_ARGV cmdline_args(cmdline); 
  for(int i=0; i<cmdline_args.argc(); ++ i)
  {
    printf("argv[%d] = %s.\n", i, cmdline_args.argv()[i]); 
  }

  ACE_Get_Opt cmd_opts(
    cmdline_args.argc(), 
    cmdline_args.argv(), 
    options, 
    0); 

  int opt = 0; 
  while((opt = cmd_opts()) != -1)
  {
    switch(opt)
    {
    case 'f':
      printf("find an option 'f' with argument: %s\n", cmd_opts.opt_arg()); 
      break; 
    case 'h':
      printf("find an option 'h' with argument: %s\n", cmd_opts.opt_arg()); 
      break; 
      break; 
    default:
      break; 
    }
  }
	return 0;
}

