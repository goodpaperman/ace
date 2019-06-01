// temperature.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Naming_Context.h" 
#include "ace/Local_Name_Space.h" 
#include "ace/Get_Opt.h" 
#include "ace/INET_Addr.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_time.h" 


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

class Thermometer
{
public:
    Thermometer (ACE_INET_Addr addr)
    {
        ACE_OS::srand (ACE_OS::thr_self ()); 
    }

    float temperature ()
    {
        return ACE_OS::rand () % 100 - 50; 
    }

    void reset ()
    {
        ACE_DEBUG ((LM_DEBUG, "Thermometer reset\n")); 
        ACE_OS::srand (ACE_OS::thr_self ()); 
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

class Temperature_Monitor
{
public:
    Temperature_Monitor (Temperature_Monitor_Options &opt, Naming_Context &ctx)
        : opt_ (opt), ctx_ (ctx) 
    {
        thermometer_ = new Thermometer (this->opt_.thermometer_address ()); 
    }

    ~Temperature_Monitor ()
    {
        delete thermometer_; 
    }

    void monitor ()
    {
        while (1)
        {
            float temp = this->thermometer_->temperature (); 
            ACE_DEBUG ((LM_INFO, "Read temperature %f\n", temp)); 

            if (temp >= 0)
                this->record_temperature (temp); 
            else 
                this->record_failure (); 

            ACE_OS::sleep (this->opt_.pool_interval ()); 
        }
    }

    void record_temperature (float temp)
    {
        Name_Binding_Ptr current (this->ctx_.fetch ("current")); 
        if (current.get ())
            this->ctx_.rebind ("previous", current->value ()); 

        this->ctx_.rebind ("current", temp); 
        this->ctx_.unbind ("lastReset"); 
        this->ctx_.unbind ("resetCount"); 
    }

    void record_failure ()
    {
        Name_Binding_Ptr lastReset (this->ctx_.fetch ("lastReset")); 
        Name_Binding_Ptr resetCount (this->ctx_.fetch ("resetCount")); 
        
        int now = ACE_OS::time (); 
        int lastResetTime = 0; 
        if (lastReset.get ())
            lastResetTime = lastReset->int_value (); 
        else 
        {
            lastResetTime = now; 
            this->ctx_.rebind ("lastReset", now); 
        }

        if (now - lastResetTime >= this->opt_.reset_interval ())
            this->reset_device (resetCount); 
    }

    void reset_device (Name_Binding_Ptr &resetCount)
    {
        int reset = 1; 
        if (resetCount.get ())
        {
            reset = resetCount->int_value () + 1; 
            if (reset >= this->opt_.excessive_resets ())
            {
                ACE_DEBUG ((LM_DEBUG, "!!! reset too many times %d\n", reset)); 
            }
        }

        this->thermometer_->reset (); 
        this->ctx_.rebind ("lastReset", (int) ACE_OS::time ()); 
        this->ctx_.rebind ("resetCount", reset); 
    }

private:
    Thermometer *thermometer_; 
    Temperature_Monitor_Options &opt_; 
    Naming_Context &ctx_; 
}; 

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    Temperature_Monitor_Options opt (argc, argv); 
    
    Naming_Context ctx; 
    ACE_Name_Options *options = ctx.name_options (); 

    char *opt_argv [] = { argv[0] }; 
    options->parse_args (1, opt_argv); 
    options->context (ACE_Naming_Context::PROC_LOCAL); 
    ctx.open (options->context ()); 

    Temperature_Monitor mnt (opt, ctx); 
    mnt.monitor (); 

    ctx.close (); 
	return 0;
}

