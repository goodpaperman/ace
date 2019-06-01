// slave.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Process.h" 
#include "ace/Log_Msg.h" 
#include "ace/os_ns_string.h" 
#include "ace/os_ns_unistd.h" 
#include "ace/os_ns_fcntl.h" 
#include "ace/os_ns_stdlib.h" 
#include "ace/os_ns_stdio.h" 
#include "ace/os_ns_pwd.h" 

class Manager : public ACE_Process
{
public: 
    Manager(ACE_TCHAR const* program_name) : outputfd_(ACE_INVALID_HANDLE) 
    {
        ACE_TRACE(ACE_TEXT("Manager::Manager")); 
        ACE_OS::strcpy(programName_, program_name); 
    }

    virtual int prepare(ACE_Process_Options &options)
    {
        ACE_TRACE(ACE_TEXT("Manager::prepare")); 
        options.command_line("%s 1", this->programName_); 
        if(this->setStdHandles(options) == -1 || 
            this->setEnvVariable(options) == -1)
            return -1; 

        if(options.pass_handle(outputfd_) == -1)
            return -1; 

#if !defined(ACE_WIN32)
        return this->setUserID(options); 
#else 
        return 0; 
#endif 
    }

    virtual void parent(pid_t child)
    {
        ACE_TRACE(ACE_TEXT("Manager::parent")); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P) child %d spawned.\n"), child)); 
    }

    virtual void child(pid_t parent)
    {
        ACE_TRACE(ACE_TEXT("Manager::child")); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P) spawned by parent %d.\n"), parent)); 
    }

    int setStdHandles(ACE_Process_Options &options)
    {
        ACE_TRACE(ACE_TEXT("Manager::setStdHandles")); 
        ACE_OS::unlink("output.txt"); 
        this->outputfd_ = ACE_OS::open("output.txt", O_RDWR | O_CREAT); 
        return options.set_handles(ACE_STDIN, ACE_STDOUT, this->outputfd_); 
    }

    int setEnvVariable(ACE_Process_Options &options)
    {
        ACE_TRACE(ACE_TEXT("Manager::setEnvVariable")); 
        return options.setenv("PRIVATE_VAR=/that/seems/to/be/it"); 
    }

#if !defined(ACE_WIN32)
    int setUserID(ACE_Process_Options &options)
    {
        ACE_TRACE(ACE_TEXT("Manager::setUserID")); 
        passwd* pw = ACE_OS::getpwnam("nobody"); 
        if(pw == 0)
            return -1; 

        options.seteuid(pw->pw_uid); 
        return 0; 
    }
#endif 

    int dumpRun()
    {
        ACE_TRACE(ACE_TEXT("Manager::dumpRun")); 
        if(ACE_OS::lseek(this->outputfd_, 0, SEEK_SET) == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("lseek")), -1); 

        char buf[1024] = { 0 }; 
        int length = 0; 
        while((length = ACE_OS::read(this->outputfd_, buf, sizeof(buf)-1)) > 0)
        {
            buf[length] = 0; 
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%C\n"), buf)); 
        }

        ACE_OS::close(this->outputfd_); 
        this->outputfd_ = ACE_INVALID_HANDLE; 
        return 0; 
    }

    int doWork()
    {
        ACE_TRACE(ACE_TEXT("Manager::doWork")); 

        ACE_Process_Options options; 
        pid_t pid = this->spawn(options); 
        if(pid == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("spawn")), -1); 

        if(this->wait() == -1)
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("wait")), -1); 

        this->dumpRun(); 
        return 0; 
    }

private:
    ACE_TCHAR programName_[MAXPATHLEN]; 
    ACE_HANDLE outputfd_; 
}; 

class Slave
{
public:
    Slave()
    {
        ACE_TRACE(ACE_TEXT("Slave::Slave")); 
    }

    void showWho (void)
    {
        ACE_TRACE (ACE_TEXT ("Slave::showWho"));
#if !defined (ACE_WIN32)
        passwd *pw = ::getpwuid (::geteuid ());
        ACE_DEBUG ((LM_INFO,
            ACE_TEXT ("(%P) Running this process as:%s\n"),
            pw->pw_name));
#endif
    }

    ACE_TCHAR* readLine (ACE_TCHAR* str)
    {
        ACE_TRACE (ACE_TEXT ("Slave::readLine"));

        int i = 0;
        while (true)
        {
            int retval = ACE_OS::read (ACE_STDIN, &str[i], 1);
            if (retval > 0)
            {
                if (str[i] == '\n')
                {
                    str[++i] = 0;
                    return str;
                }
                i++;
            }
            else
                return str;
        }
    }

    int doWork()
    {
        ACE_TRACE(ACE_TEXT("Slave::doWork")); 
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P) started at %T, parent is %d\n"), ACE_OS::getppid())); 
        this->showWho(); 
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P) the private environment is %s\n"), ACE_OS::getenv("PRIVATE_VAR"))); 

        ACE_TCHAR str[128] = { 0 }; 
        ACE_OS::sprintf(str, ACE_TEXT("(%d) Enter your command\n"), ACE_OS::getpid()); 
        ACE_OS::write(ACE_STDOUT, str, ACE_OS::strlen(str)); 
        this->readLine(str); 
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P) Executed: %C\n"), str)); 
        return 0; 
    }
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if(argc > 1)
    {
        Slave s; 
        return s.doWork(); 
    }

    Manager m(argv[0]); 
    return m.doWork(); 
}

