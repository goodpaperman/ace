// command.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Stream.h" 
#include "ace/SString.h" 
#include "ace/SOCK_Stream.h" 

class Command : public ACE_Data_Block
{
public:
    enum { 
        PASS = 1, 
        SUCCESS = 0, 
        FAILURE = -1
    }; 

    enum {
        UNKNOWN = -1, 
        ANSWER_CALL = 10, 
        RETRIEVE_CALLER_ID, 
        PLAY_MESSAGE, 
        RECORD_MESSAGE, 
    } commands; 

    int flags_; 
    int cmd_; 

    void *args_; 

    int num_res_; 
    ACE_CString str_res_; 
}; 



class CommandTask; 
class CommandModule : public ACE_Module <ACE_MT_SYNCH>
{
public:
    typedef ACE_Module <ACE_MT_SYNCH> BASE; 
    typedef ACE_Task <ACE_MT_SYNCH> TASK; 

    CommandModule (ACE_TCHAR const* name, 
        CommandTask *writer, 
        CommandTask *reader, 
        ACE_SOCK_Stream *peer)
        : BASE (name, (TASK*)writer, (TASK*)reader, peer)
    {
    }

    ACE_SOCK_Stream &peer ()
    {
        ACE_SOCK_Stream *peer = (ACE_SOCK_Stream *)this->arg (); 
        return *peer; 
    }
}; 



class CommandTask : public ACE_Task <ACE_MT_SYNCH>
{
public:
    typedef ACE_Task <ACE_MT_SYNCH> BASE; 
    
    virtual ~CommandTask () { }

    virtual int open (void *args = 0) {
        return this->activate (); 
    }

    int put (ACE_Message_Block *msg, ACE_Time_Value *tv = 0) {
        return this->putq (msg, tv); 
    }

    CommandModule * module () const {
        return (CommandModule *)BASE::module (); 
    }

    //virtual int close (u_long flags = 0); 

    virtual int svc ()
    {
        int ret = 0; 
        ACE_Message_Block *mb = 0; 
        Command *cmd = 0; 

        while (1)
        {
            ret = this->getq (mb); 
            if (ret == -1)
                ACE_ERROR_RETURN ((LM_ERROR, "%t: %p\n", "getq"), -1); 

            cmd = (Command *)mb->data_block (); 
            ACE_DEBUG ((LM_DEBUG, 
                "%s got work request %d\n", 
                this->module ()->name (), 
                cmd->cmd_)); 

            if (cmd->cmd_ != this->cmd_)
                this->put_next (mb); 
            else 
            {
                ret = this->process (cmd); 
                ACE_DEBUG ((LM_DEBUG, 
                    "%s work request %d result is %d\n", 
                    this->module ()->name (), 
                    cmd->cmd_, 
                    ret)); 

                if (ret == Command::FAILURE)
                    cmd->num_res_ = -1; 
                else if (ret == Command::PASS)
                    this->put_next (mb); 
                else // SUCCESS
                {
                    if (this->is_writer ())
                        //this->reply (msg); 
                        this->sibling ()->putq (mb); 
                    else  // is_reader
                        this->put_next (mb); 
                }
            }
        }

        return 0; 
    }

protected:
    CommandTask (int cmd)
        : cmd_ (cmd)
    {
    }

    virtual int process (Command *cmd) {
        return Command::PASS; 
    }

    int cmd_; 
}; 




class AnswerCallDownStreamTask : public CommandTask 
{
public:
    AnswerCallDownStreamTask ()
        : CommandTask (Command::ANSWER_CALL)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Answer Call (downstream)\n")); 
        cmd->num_res_ = 0; 
        return Command::SUCCESS; 
    }
}; 

class AnswerCallUpStreamTask : public CommandTask 
{
public:
    AnswerCallUpStreamTask ()
        : CommandTask (Command::ANSWER_CALL)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Answer Call (upstream)\n")); 
        return Command::SUCCESS; 
    }
}; 

class AnswerCallModule : public CommandModule
{
public:
    AnswerCallModule (ACE_SOCK_Stream *peer)
        : CommandModule ("AnswerCall Module", 
        new AnswerCallDownStreamTask (), 
        new AnswerCallUpStreamTask (), 
        peer )
    {
    }
}; 




class RetrieveCallerIdDownStreamTask : public CommandTask 
{
public:
    RetrieveCallerIdDownStreamTask ()
        : CommandTask (Command::RETRIEVE_CALLER_ID)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Retrieve Caller Id (downstream)\n")); 
        return Command::SUCCESS; 
    }
}; 

class RetrieveCallerIdUpStreamTask : public CommandTask 
{
public:
    RetrieveCallerIdUpStreamTask ()
        : CommandTask (Command::RETRIEVE_CALLER_ID)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Retrieve Caller Id (upstream)\n")); 
        cmd->num_res_ = 1; 
        return Command::SUCCESS; 
    }
}; 

class RetrieveCallerIdModule : public CommandModule
{
public:
    RetrieveCallerIdModule (ACE_SOCK_Stream *peer)
        : CommandModule ("RetrieveCallerId Module", 
        new RetrieveCallerIdDownStreamTask (), 
        new RetrieveCallerIdUpStreamTask (), 
        peer )
    {
    }
}; 




class PlayMessageDownStreamTask : public CommandTask 
{
public:
    PlayMessageDownStreamTask ()
        : CommandTask (Command::PLAY_MESSAGE)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Play message (downstream)\n")); 
        return Command::SUCCESS; 
    }
}; 

class PlayMessageUpStreamTask : public CommandTask 
{
public:
    PlayMessageUpStreamTask ()
        : CommandTask (Command::PLAY_MESSAGE)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Play message (upstream)\n")); 
        cmd->num_res_ = 2; 
        return Command::SUCCESS; 
    }
}; 

class PlayMessageModule : public CommandModule
{
public:
    PlayMessageModule (ACE_SOCK_Stream *peer)
        : CommandModule ("PlayMessage Module", 
        new PlayMessageDownStreamTask (), 
        new PlayMessageUpStreamTask (), 
        peer)
    {
    }
}; 




class RecordMessageDownStreamTask : public CommandTask 
{
public:
    RecordMessageDownStreamTask ()
        : CommandTask (Command::RECORD_MESSAGE)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Record message (downstream)\n")); 
        return Command::SUCCESS; 
    }
}; 

class RecordMessageUpStreamTask : public CommandTask 
{
public:
    RecordMessageUpStreamTask ()
        : CommandTask (Command::RECORD_MESSAGE)
    {
    }

protected:
    virtual int process (Command *cmd)
    {
        ACE_DEBUG ((LM_DEBUG, "Record message (upstream)\n")); 
        cmd->str_res_ = "Hello World!"; 
        cmd->num_res_ = -1; 
        return Command::SUCCESS; 
    }
}; 

class RecordMessageModule : public CommandModule
{
public:
    RecordMessageModule (ACE_SOCK_Stream *peer)
        : CommandModule ("RecordMessage Module", 
        new RecordMessageDownStreamTask (), 
        new RecordMessageUpStreamTask (), 
        peer)
    {
    }
}; 



class CommandStream : public ACE_Stream <ACE_MT_SYNCH> 
{
public: 
    typedef ACE_Stream <ACE_MT_SYNCH> BASE; 
    typedef ACE_Module <ACE_MT_SYNCH> MODULE; 

    CommandStream () : BASE () { }

    virtual int open (ACE_SOCK_Stream *peer)
    {
        CommandModule *p[4] = { 0 }; 
        ACE_NEW_RETURN (p[0], AnswerCallModule (peer), -1); 
        ACE_NEW_RETURN (p[1], RetrieveCallerIdModule (peer), -1); 
        ACE_NEW_RETURN (p[2], PlayMessageModule (peer), -1); 
        ACE_NEW_RETURN (p[3], RecordMessageModule (peer), -1); 

        for (int i=4; i>0; -- i)
            if (this->push (p[i-1]) == -1)
                ACE_ERROR_RETURN ((LM_ERROR, "Failed to push module %s\n", p[i]->name ()), -1); 

        return 0; 
    }

    Command* execute (Command *cmd)
    {
        ACE_Message_Block *mb = 0; 
        ACE_NEW_RETURN (mb, ACE_Message_Block (cmd), 0); 
        if (this->put (mb) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "Fail to put command %d: %p\n", cmd->cmd_), 0); 

        if (this->get (mb) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "Fail to get command %d return: %p\n", cmd->cmd_), 0); 

        // actually command == cmd
        Command *command = (Command *)mb->data_block (); 
        ACE_DEBUG ((LM_DEBUG, "Command (%d) returns (%d)\n", cmd->cmd_, cmd->num_res_)); 
        return command; 
    }
}; 



class TextListener
{
public:
    TextListener ()
        : peer_ () 
    {
        ACE_NEW (this->stream_, CommandStream ()); 
        this->stream_->open (&this->peer_); 
    }

    ~TextListener ()
    {
        this->stream_->close (1); 
        delete this->stream_; 
    }

    int answer_call ()
    {
        ACE_DEBUG ((LM_DEBUG, "answer_call ()\n")); 
        Command *c = new Command (); 
        c->cmd_ = Command::ANSWER_CALL; 
        //c->args_ = this; 
        c = this->stream_->execute (c); 

        int ret = c->num_res_; 
        delete c; 

        ACE_DEBUG ((LM_DEBUG, "result = %d\n", ret)); 
        return ret; 
    }

    int retrieve_caller_id ()
    {
        ACE_DEBUG ((LM_DEBUG, "retrieve_caller_id ()\n")); 
        Command *c = new Command (); 
        c->cmd_ = Command::RETRIEVE_CALLER_ID; 
        //c->args_ = this; 
        c = this->stream_->execute (c); 

        int ret = c->num_res_; 
        delete c; 

        ACE_DEBUG ((LM_DEBUG, "result = %d\n", ret)); 
        return ret; 
    }

    int play_message ()
    {
        ACE_DEBUG ((LM_DEBUG, "play_message ()\n")); 
        Command *c = new Command (); 
        c->cmd_ = Command::PLAY_MESSAGE; 
        //c->args_ = this; 
        c = this->stream_->execute (c); 

        int ret = c->num_res_; 
        delete c; 

        ACE_DEBUG ((LM_DEBUG, "result = %d\n", ret)); 
        return ret; 
    }

    ACE_CString record_message ()
    {
        ACE_DEBUG ((LM_DEBUG, "record_message ()\n")); 
        Command *c = new Command (); 
        c->cmd_ = Command::RECORD_MESSAGE; 
        //c->args_ = this; 
        c = this->stream_->execute (c); 

        ACE_CString ret = c->str_res_; 
        delete c; 

        ACE_DEBUG ((LM_DEBUG, "result = %s\n", ret.c_str ())); 
        return ret; 
    }


protected:
    ACE_SOCK_Stream peer_; 
    CommandStream *stream_; 
}; 


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    TextListener tl; 
    tl.answer_call (); 
    tl.retrieve_caller_id (); 
    tl.play_message (); 
    tl.record_message (); 
	return 0;
}

