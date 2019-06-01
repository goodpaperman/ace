// namesvc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Naming_Context.h" 
#include "ace/OS_NS_stdio.h" 

#define CLEANUP

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    ACE_Naming_Context ctx; 
    //ctx.name_options ()->namespace_dir ("C:/"); 
    //ctx.name_options ()->process_name (argv[0]); 

    //char const* basename = ACE::basename (argv[0]); 
    //char const* p = ACE_OS::strrchr (basename, '.'); 
    //if (p)
    //    *(char*)p = 0; 

    //ctx.name_options ()->database (basename); 

    ctx.name_options ()->parse_args (argc, argv); 
    if (ctx.open (ctx.name_options ()->context ()) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "context open\n"), -1); 

    char name[128] = { 0 }; 
    char value[128] = { 0 }; 
    for (int i=0; i<100; ++ i)
    {
        ACE_OS::sprintf (name, "hello_%d", i); 
        ACE_OS::sprintf (value, "world_%d", i); 
        if (ctx.bind (name, value, "!!") == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "bind"), -1); 
    }

    {
        ACE_NS_WString value_out; 
        char *type = 0; 
        if (ctx.resolve ("hello_33", value_out, type) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "resolve"), -1); 

        char *rep = value_out.char_rep (); 
        ACE_DEBUG ((LM_DEBUG, 
            "value = '%s'\n"
            "type = '%s'\n", 
            rep, 
            type)); 

        delete []rep; 
    }

    {
        ACE_BINDING_SET names; 
        if (ctx.list_type_entries (names, "!!") == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "list_type_entries"), -1); 

        int n = 0; 
        char *rep_name = 0, *rep_value; 
        ACE_Name_Binding *name = 0; 
        for (ACE_BINDING_SET::iterator it (names); 
            it.next (name); it.advance ())
        {
            rep_name = name->name_.char_rep (); 
            rep_value = name->value_.char_rep (); 
            ACE_DEBUG ((LM_DEBUG, "%d: %s, %s, %s\n", n++, rep_name, rep_value, name->type_)); 
            delete []rep_name; 
            delete [] rep_value; 
        }
    }

    {
        ACE_WSTRING_SET names; 
        if (ctx.list_names (names, "hello") == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "list_names"), -1); 

        int n = 0; 
        char *rep = 0; 
        ACE_NS_WString *name = 0; 
        for (ACE_WSTRING_SET::iterator it (names); 
            it.next (name); it.advance ())
        {
            rep = name->char_rep (); 
            ACE_DEBUG ((LM_DEBUG, "%d: %s\n", n++, rep)); 
            delete []rep; 

#if defined (CLEANUP)
            if (ctx.unbind (*name) == -1)
                ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "unbind"), -1); 
#endif 
        }
    }

#if defined (CLEANUP)
    {
        ACE_NS_WString value_out; 
        char *type = 0; 
        if (ctx.resolve ("hello_33", value_out, type) == 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "resolve"), -1); 

        ACE_DEBUG ((LM_DEBUG, "hello_33 not find!\n")); 
    }
#endif 

    {
        ACE_OS::sprintf (name, "this\\is\\a\\test\\to\\see\\registry\\storage"); 
        ACE_OS::sprintf (value, "yeah\\I\\got\\it"); 
        if (ctx.bind (name, value, "?") == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "bind"), -1); 

        ACE_NS_WString value_out; 
        char *type = 0; 
        if (ctx.resolve (name, value_out, type) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "resolve"), -1); 

        char *rep = value_out.char_rep (); 
        ACE_DEBUG ((LM_DEBUG, "find! %s %s\n", rep, type)); 
        delete [] rep; 

#if defined (CLEANUP)
        if (ctx.unbind (name) == -1)
            ACE_ERROR_RETURN ((LM_DEBUG, "%p\n", "unbind"), -1); 
#endif 
    }

    ctx.close (); 
    return 0;
}

