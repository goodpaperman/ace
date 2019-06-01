// tempclnt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Naming_Context.h" 
#include "ace/Local_Name_Space.h" 
#include "ace/Get_Opt.h" 
#include "ace/INET_Addr.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_time.h" 

#include <list> 
#include <algorithm> 
using std::list; 

#define USE_NET


class Name_Binding
{
public:
    Name_Binding (ACE_Name_Binding *entry)
    {
        this->name_ = entry->name_.char_rep (); 
        this->value_ = entry->value_.char_rep (); 
        this->type_ = ACE_OS::strdup (entry->type_); 
    }

    Name_Binding (ACE_NS_String const& n, 
        ACE_NS_String const& v, 
        char const* t)
    {
        this->name_ = n.char_rep (); 
        this->value_ = v.char_rep (); 
        this->type_ = t; //ACE_OS::strdup (t); 
    }

    ~Name_Binding ()
    {
        delete [] this->name_; 
        delete [] this->value_; 
        delete [] this->type_; 
    }

    char *name () { return this->name_; } 
    char *value () { return this->value_; } 
    char const* type () { return this->type_; } 
    int int_value () { return ACE_OS::atoi (this->value ()); } 


private:
    char *name_; 
    char *value_; 
    char const* type_; 
}; 

typedef auto_ptr <Name_Binding> Name_Binding_Ptr; 

class Graphable_Element : public Name_Binding
{
public:
    Graphable_Element (ACE_Name_Binding *entry)
        :Name_Binding (entry)
    {
        sscanf (this->value (), "%d|%f", &this->when_, &this->temp_); 
    }

    inline int when () { return this->when_; } 
    inline float temp () { return this->temp_; } 
    inline bool operator < (Graphable_Element &other)
    {
        return this->when () < other.when (); 
    }

private:
    int when_; 
    float temp_; 
}; 

typedef std::list <Graphable_Element*> Graphable_Element_List; 


class Naming_Context : public ACE_Naming_Context 
{
public:
    typedef ACE_Naming_Context BASE; 

    int rebind (char const* name_in, 
        char const* value_in, 
        char const* type_in = "")
    {
        return this->BASE::rebind (name_in, value_in, type_in); 
    }

    int rebind (char const* name_in, 
        float value_in, 
        char const* type_in = "")
    {
        char buf[BUFSIZ] = { 0 }; 
        ACE_OS::sprintf (buf, "%2f", value_in); 
        return this->BASE::rebind (name_in, buf, type_in); 
    }

    int rebind (char const* name_in, 
        int value_in, 
        char const* type_in = "")
    {
        char buf[BUFSIZ] = { 0 }; 
        ACE_OS::sprintf (buf, "%d", value_in); 
        return this->BASE::rebind (name_in, buf, type_in); 
    }

    Name_Binding *fetch (char const* name)
    {
        ACE_NS_WString value; 
        char *type = 0; 
        if (this->resolve (name, value, type) != 0 ||
            value.length () < 1)
            return 0; 

        Name_Binding *rval = new Name_Binding (ACE_NS_String (name), value, type); 
        return rval; 
    }
}; 

class Temperature_Monitor_Options 
{
public:
    Temperature_Monitor_Options (int argc, ACE_TCHAR *argv[])
        : addr_ ()
        , interval_ (1) 
        , reset_ (4) 
        , max_ (1) 
    {
        parse_args (argc, argv); 
    }

    int parse_args (int argc, ACE_TCHAR *argv[])
    {
        int c = 0; 
        char host[MAXHOSTNAMELEN] = { "localhost" }; 
        u_short port = 10003; 
        ACE_Get_Opt opt (argc, argv, "a:p:i:r:m:"); 
        while ((c = opt ()) != -1)
        {
            switch (c)
            {
            case 'a':
                ACE_OS::strcpy (host, opt.opt_arg ()); 
                break; 
            case 'p':
                port = ACE_OS::atoi (opt.opt_arg ()); 
                break; 
            case 'i':
                interval_ = ACE_OS::atoi (opt.opt_arg ()); 
                break; 
            case 'r':
                reset_ = ACE_OS::atoi (opt.opt_arg ()); 
                break; 
            case 'm':
                max_ = ACE_OS::atoi (opt.opt_arg ()); 
                break; 
            default:
                ACE_DEBUG ((LM_DEBUG, "unknown %c\n", c)); 
                break; 
            }
        }

        addr_.set (host, port); 
        ACE_DEBUG ((LM_DEBUG, "interval = %d, reset = %d, max = %d\n", interval_, reset_, max_)); 
        return 0; 
    }

    ACE_INET_Addr thermometer_address () { return addr_; } 
    int pool_interval () { return interval_; } 
    int reset_interval () { return reset_; } 
    int excessive_resets () { return max_; } 

private:
    ACE_INET_Addr addr_; 
    int interval_; 
    int reset_; 
    int max_; 
}; 

class Graph
{
public:
    void graph (time_t stamp, Graphable_Element_List &list)
    {
        char buf[BUFSIZ] = { 0 }; 
        ACE_OS::strftime (buf, BUFSIZ, "%Y-%m-%d %H:%M:%S\n", localtime (&stamp)); 
        ACE_DEBUG ((LM_DEBUG, "timestamp: %s\n", buf)); 

        int n = 0; 
        while (!list.empty ())
        {
            Graphable_Element *ge = list.front (); 
            stamp = ge->when (); 

#if defined (SHOW_STAMP)
            ACE_OS::strftime (buf, BUFSIZ, "%Y-%m-%d %H:%M:%S\n", localtime (&stamp)); 
            ACE_DEBUG ((LM_DEBUG, "%-3d %s ", n++, buf)); 
#else 
            ACE_DEBUG ((LM_DEBUG, "%-3d ", n++)); 
#endif 

            for (int i=0; i<ge->temp (); ++ i)
                ACE_DEBUG ((LM_DEBUG, "=")); 

            ACE_DEBUG ((LM_DEBUG, "\n")); 
            list.pop_front (); 
            delete ge; 
        }
    }
}; 

int ge_pred (Graphable_Element* lhs, Graphable_Element *rhs)
{
    return lhs->when () < rhs->when (); 
}

class Temperature_Grapher
{
public:
    Temperature_Grapher (Temperature_Monitor_Options &opt, Naming_Context &ctx)
        : opt_ (opt), ctx_ (ctx) 
    {
    }

    ~Temperature_Grapher ()
    {
    }

    void monitor ()
    {
        while (1)
        {
            this->update_graph (); 
            ACE_OS::sleep (this->opt_.pool_interval ()); 
        }
    }

    void update_graph ()
    {
        Name_Binding_Ptr lastUpdate (this->ctx_.fetch ("lastUpdate")); 
        if (!lastUpdate.get ())
        {
            ACE_DEBUG ((LM_DEBUG, "No data to graph \n")); 
            return; 
        }

        Name_Binding_Ptr lastGraphed (this->ctx_.fetch ("lastGraphed")); 
        if (lastGraphed.get () && 
            lastGraphed->int_value () == lastUpdate->int_value ())
        {
            ACE_DEBUG ((LM_DEBUG, "Data already graphed\n")); 
            return; 
        }

        ACE_BINDING_SET set; 
        if (this->ctx_.list_name_entries (set, "history[") != 0)
        {
            ACE_DEBUG ((LM_INFO, "There's nothing to graph\n")); 
            return; 
        }

        Graphable_Element_List list; 
        ACE_BINDING_ITERATOR iter (set); 
        for (ACE_Name_Binding *entry = 0; 
            iter.next (entry) != 0; 
            iter.advance ())
        {
            Name_Binding binding (entry); 
            ACE_DEBUG ((LM_DEBUG, "%s\t%s\t%s\n", 
                binding.type (), 
                binding.name (), 
                binding.value ())); 

            Graphable_Element *ge = new Graphable_Element (entry); 
            list.push_back (ge); 
        }

        //std::sort (list.begin (), list.end (), ge_pred); 
        list.sort (ge_pred); 
        g_.graph (lastUpdate->int_value (), list); 
        this->ctx_.rebind ("lastGraphed", lastUpdate->int_value ()); 
    }

private:
    Temperature_Monitor_Options &opt_; 
    Naming_Context &ctx_; 
    Graph g_;
}; 

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    Temperature_Monitor_Options opt (argc, argv); 
    
    Naming_Context ctx; 
    ACE_Name_Options *options = ctx.name_options (); 

#if defined (USE_NET)
    options->nameserver_host ("127.0.0.1"); 
    options->nameserver_port (20012); 
    options->context (ACE_Naming_Context::NET_LOCAL); 
#else
    options->database ("temperature"); 
    //options->process_name (argv[0]); 
    options->base_address ((char *)0x8000000); 
    options->context (ACE_Naming_Context::NODE_LOCAL); 
#endif 

    ctx.open (options->context ()); 

    Temperature_Grapher gph (opt, ctx); 
    gph.monitor (); 

    ctx.close (); 
	return 0;
}

