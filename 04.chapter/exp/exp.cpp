// exp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Configuration.h" 
#include "ace/Configuration_Import_Export.h" 


int _tmain(int argc, _TCHAR* argv[])
{
  HKEY hkey = 0; 
  if(RegOpenKey(HKEY_LOCAL_MACHINE, ACE_TEXT("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application"), &hkey) != ERROR_SUCCESS)
  {
    printf("error = %d\n", GetLastError()); 
    ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("%p\n"), ACE_TEXT("open registry key")), -1); 
  }

  ACE_Configuration_Win32Registry config(hkey); 
#ifdef TEST_REGISTRY_FORMAT
  ACE_Registry_ImpExp exporter(config); 
#else 
  ACE_Ini_ImpExp exporter(config); 
#endif 
  if(exporter.export_config("C:\\Documents and Settings\\Administrator.BFCFECD7B00A4C3\\My Documents\\registry2.reg") != 0)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("export config failed\n")), -1); 

	return 0;
}

