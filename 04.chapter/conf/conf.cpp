// conf.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Configuration.h" 
#include "ace/Configuration_Import_Export.h" 


int _tmain(int argc, _TCHAR* argv[])
{
  ACE_TCHAR config_file[MAXPATHLEN] = { ACE_TEXT("C:\\config.ini") }; 
  ACE_Configuration_Heap config; 
  if(config.open() == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("config")), -1); 

  ACE_Registry_ImpExp config_importer(config); 
  if(config_importer.import_config(config_file) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), config_file), -1); 

  ACE_Configuration_Section_Key status_section; 
  if(config.open_section(config.root_section(), ACE_TEXT("HAStatus"), 0, status_section) == -1)
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("Can't open HAStatus section")), -1); 

  u_int status_port; 
  if(config.get_integer_value(status_section, ACE_TEXT("ListenPort"), status_port) == -1)
  {
    if(config.set_integer_value(status_section, ACE_TEXT("ListenPort"), 9999) == -1)
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("set ListenPort failed.\n"))); 
    else
    {
      ACE_Configuration_Section_Key basic_section; 
      if(config.open_section(status_section, ACE_TEXT("basic"), 1, basic_section) == -1)
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("open section failed.\n"))); 
      else 
      {
        if(config.set_string_value(basic_section, ACE_TEXT("settings"), ACE_TEXT("normal")) == -1)
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("set string value failed.\n"))); 

        ACE_Configuration_Section_Key network; 
        if(config.open_section(basic_section, ACE_TEXT("network"), 1, network) == -1)
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("open network failed.\n"))); 
      }

      ACE_Registry_ImpExp config_exporter(config); 
      if(config_exporter.export_config(config_file) == -1)
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("export config failed.\n"))); 
    }

    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("HAStatus ListenPort does not exist\n")), -1); 
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("listen port = %d\n"), status_port)); 
	return 0;
}

