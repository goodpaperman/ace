// getopt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Get_Opt.h" 
#include "ace/os_ns_string.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  static const ACE_TCHAR options[] = ACE_TEXT(":abd:f:W;"); 
  ACE_Get_Opt cmd_opts(argc, argv, options, 1, 1, ACE_Get_Opt::RETURN_IN_ORDER, 1); 
  if(cmd_opts.long_option(ACE_TEXT("config"), 'f', ACE_Get_Opt::ARG_REQUIRED) == -1)
  {
    ACE_DEBUG((LM_EMERGENCY, ACE_TEXT("Stupid, Wrong!\n"))); 
    return -1; 
  }

  if(cmd_opts.long_option(ACE_TEXT("cool_option")) == -1)
  {
    ACE_DEBUG((LM_EMERGENCY, ACE_TEXT("Stupid, Wrong!\n"))); 
    return -1; 
  }

  if(cmd_opts.long_option(ACE_TEXT("the_answer"), 'A') == -1)
  {
    ACE_DEBUG((LM_EMERGENCY, ACE_TEXT("Stupid, Wrong!\n"))); 
    return -1; 
  }

  int option = 0; 
  ACE_TCHAR config_file[MAXPATHLEN] = { 0 }; 
  ACE_OS_String::strcpy(config_file, ACE_TEXT("HAStatus.conf")); 
  while((option = cmd_opts()) != EOF)
  {
    switch(option)
    {
    case 'a':
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("find option a\n"))); 
      break; 
    case 'b':
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("find option b\n"))); 
      break; 
    case 'd':
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("find option d, arg = %s\n"), cmd_opts.opt_arg())); 
      break; 
    case 'f':
      ACE_OS_String::strncpy(config_file, cmd_opts.opt_arg(), MAXPATHLEN); 
      break; 
    case ':':
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("we got an ':', means -%c requries an argument\n"), cmd_opts.opt_opt()), -1); 
    case '?':
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("we got an '?', means -%c requries an argument\n"), cmd_opts.opt_opt()), -1); 
    case '#':
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("we got an '#', means -%c requries an argument\n"), cmd_opts.opt_opt()), -1); 
    case 0:
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s, Yes, very cool.\n"), cmd_opts.long_option())); 
      break; 
    case 1:
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("encount an argument %s.\n"), cmd_opts.opt_arg())); 
      //++ cmd_opts.opt_ind(); 
      continue; 
    case 'A':
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("the answer is 'A'\n"))); 
      break; 
    default:
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Parse error.\n")), -1); 
    }
  }

  if(cmd_opts.opt_ind() < argc)
  {
    ACE_DEBUG((LM_NOTICE, ACE_TEXT("we leave something: %s\n"), cmd_opts.argv()[cmd_opts.opt_ind()])); 
  }

  ACE_DEBUG((LM_INFO, ACE_TEXT("file = %s\n"), config_file)); 
  ACE_DEBUG((LM_INFO, ACE_TEXT("option string = %s\n"), cmd_opts.optstring())); 
	return 0;
}

