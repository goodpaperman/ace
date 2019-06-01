// tokenizer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/SString.h" 
#include "ace/Log_Msg.h" 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  ACE_TCHAR* p = 0; 
  ACE_TCHAR strbuf[] = ACE_TEXT("dreamgirl/(outspoken/caveman)/goodcitizen/nightmare/fly(paper/paperman/middle)man/papertiger/paper(air)plane"); 
  ACE_Tokenizer token(strbuf); 

  // case 1
  //token.delimiter('/'); 
  //
  // case 2
  //token.delimiter_replace('/', '\0'); 
  //
  // case 3 
  token.delimiter_replace('/', '\0'); 
  token.preserve_designators('(', ')'); 
  //
  // case 4 
  //token.delimiter('/'); 
  //token.preserve_designators('(', ')', 0); 
  // 
  // case 5 
  //token.delimiter('/'); 
  //token.preserve_designators('(', ')'); 
  // 
  // case 6
  //token.delimiter_replace('/', '\0'); 
  //token.preserve_designators('(', ')', 0); 

  while((p = token.next()) != 0)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s\n"), p)); 
  }

	return 0;
}

