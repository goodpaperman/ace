// caller.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Stream.h" 
#include "ace/Semaphore.h" 
#include "ace/FILE_Addr.h" 
#include "ace/FILE_IO.h" 
#include "ace/FILE_Connector.h" 
#include "ace/SString.h" 
#include "ace/OS_NS_unistd.h" 


struct CallerID
{
    int id; 
    int string (char *buf) { 
        return ACE_OS::sprintf (buf, "%d", id); 
    }
}; 

struct MessageType
{
    int value; 
}; 

class RecordingDevice
{
public: 
    RecordingDevice ()
        : sema ()
    {
    }

    RecordingDevice* wait_for_activity ()
    {
        ACE_OS::sleep (1); 
        //ACE_OS::sleep (ACE_Time_Value (0, 1000)); 
        sema.acquire (); 
        return this; 
    }
    
    virtual int answer_call () 
    {
        ACE_DEBUG ((LM_INFO, "%t: answer incoming call\n")); 
        return 0; 
    }

    virtual CallerID* retrieve_caller_id ()
    {
        ACE_DEBUG ((LM_INFO, "%t: get caller ID\n")); 
        CallerID *id = new CallerID; 
        id->id = 1234; 
        return id; 
    }

    virtual int play_message (ACE_FILE_Addr const& addr)
    {
        ACE_DEBUG ((LM_INFO, "%t: play outgoing message @%s\n", addr.get_path_name ())); 
        ACE_FILE_IO stream; 
        ACE_FILE_Connector conn; 
        if (conn.connect (stream, addr) < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), -1); 

        int len = 0; 
        char buf[1024] = { 0 }; 
        len = stream.recv (buf, 1023); 
        if (len == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "recv"), -1); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, buf, len + 1); 
        stream.close (); 
        return 0; 
    }

    virtual MessageType* record_message (ACE_FILE_Addr const& addr)
    {
        ACE_DEBUG ((LM_INFO, "%t: record incoming message\n")); 
        ACE_FILE_IO stream; 
        ACE_FILE_Connector conn; 
        if (conn.connect (stream, addr) < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), 0); 

        int len = 0; 
        char buf[1024] = { 0 }; 
        len = ACE_OS::sprintf (buf, "叫你们技术部来接下电话！！！"); 
        len = stream.send (buf, len+1); 
        if (len == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "send"), 0); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, buf, len + 1); 
        stream.close (); 

        MessageType *type = new MessageType; 
        type->value = 1;  // is mp3
        return type; 
    }

    virtual int release () 
    {
        ACE_DEBUG ((LM_INFO, "%t: release device\n")); 
        sema.release (); 
        return 0; 
    }

    virtual int save_meta_data (ACE_FILE_Addr const& addr)
    {
        ACE_DEBUG ((LM_INFO, "save meta-data\n")); 
        ACE_FILE_IO stream; 
        ACE_FILE_Connector conn; 
        ACE_CString path (addr.get_path_name ()); 
        path += ".xml"; 
        if (conn.connect (stream, ACE_FILE_Addr (path.c_str ())) < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), 0); 

        stream.truncate (0); 

        int len = 0; 
        char buf[1024] = { 0 }; 
        len = ACE_OS::sprintf (buf, 
            "<Message>\n"
            "</Message>\n"); 

        len = stream.send (buf, len+1); 
        if (len == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "send"), 0); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, buf, len + 1); 
        stream.close (); 
        return 0; 
    }

private:
    ACE_Semaphore sema; 
}; 

class RecordingDeviceFactory 
{
public:
    static RecordingDevice *instantiate (int argc, ACE_TCHAR *argv[])
    {
        return new RecordingDevice (); 
    }
}; 

struct Message 
{
    RecordingDevice *recorder; 
    CallerID *caller_id; 
    MessageType *type; 
    ACE_FILE_Addr io; 
    ACE_FILE_Addr dest; 
}; 

class BasicTask : public ACE_Task <ACE_MT_SYNCH> 
{
public:
    typedef ACE_Task <ACE_MT_SYNCH> BASE; 
    BasicTask () : BASE ()
    {
    }

    virtual int desired_threads () const 
    {
        return 1; 
    }

    virtual int open (void *args = 0)
    {
        ACE_DEBUG ((LM_DEBUG, "BasicTask starting %d threads\n", this->desired_threads ())); 
        return this->activate (THR_NEW_LWP | THR_JOINABLE, this->desired_threads ()); 
    }

    // NOTE this relay is very IMPORTANT !!!
    virtual int put (ACE_Message_Block *mb, ACE_Time_Value *tv = 0)
    {
        return putq (mb, tv); 
    }

    virtual int svc ()
    {
        ACE_Message_Block *mb = 0; 
        while (1)
        {
            if (this->getq (mb) == -1)
                ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "getq"), -1); 

            if (mb->msg_type () == ACE_Message_Block::MB_HANGUP)
            {
                if (this->putq (mb) == -1)
                {
                    mb->release (); 
                    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "putq"), -1); 
                }

                break; 
            }

            Message *msg = (Message *)mb->rd_ptr (); 
            if (this->process (msg) == -1)
            {
                mb->release (); 
                ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "process"), -1); 
            }

            //ACE_DEBUG ((LM_DEBUG, "%t continue to next stage.\n")); 
            if (this->next_step (mb) < 0)
            {
                mb->release (); 
                ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "put_next"), -1); 
            }
        }

        return 0; 
    }

    virtual int process (Message *)
    {
        ACE_DEBUG ((LM_INFO, "general message process does nothing\n")); 
        return 0; 
    }

    virtual int next_step (ACE_Message_Block *msg)
    {
        //ACE_DEBUG ((LM_INFO, "general send message to next module.\n")); 
        return this->put_next (msg); 
    }
}; 

class EndTask : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        ACE_DEBUG ((LM_INFO, "end message process\n")); 
        delete msg->caller_id; 
        delete msg->type; 
        return 0; 
    }

    virtual int next_step (ACE_Message_Block *msg)
    {
        ACE_DEBUG ((LM_INFO, "end of the line.\n")); 
        msg->release (); 
        return 0; 
    }
}; 

class AnswerIncomingCall : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        if (msg->recorder->answer_call () < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "AnswerIncomingCall"), -1); 

        return 0; 
    }
}; 

class GetCallerId : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        msg->caller_id =  msg->recorder->retrieve_caller_id (); 
        if (msg->caller_id == 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "GetCallerId"), -1); 
        
        return 0; 
    }
}; 

class PlayOutgoingMessage : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        ACE_FILE_Addr addr; 
        addr.set (get_message_addr ()); 

        int ret = msg->recorder->play_message (addr); 
        if (ret < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "PlayOutgoingMessage"), -1); 

        return 0; 
    }

    virtual char const* get_message_addr ()
    {
        return "c:/list.ini"; 
    }
}; 

class RecordIncomingMessage : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        ACE_FILE_Addr addr (this->get_incoming_message_queue ()); 
        MessageType *type = msg->recorder->record_message (addr); 
        if (type == 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "RecordIncomingMessage"), -1); 

        msg->io = addr; 
        msg->type = type; 
        return 0; 
    }

    virtual char const* get_incoming_message_queue ()
    {
        return "c:/speech.txt"; 
    }
}; 

class ReleaseDevice : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        msg->recorder->release (); 
        return 0; 
    }
}; 

namespace Util
{
    int convert_to_common (ACE_FILE_Addr &io, ACE_FILE_Addr &addr)
    {
        ACE_FILE_IO stream; 
        ACE_FILE_Connector conn; 
        if (conn.connect (stream, io) < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), 0); 

        int len = 0; 
        char buf[1024] = { 0 }; 
        len = stream.recv (buf, 1023); 
        if (len == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "recv"), 0); 

        stream.close (); 
        if (conn.connect (stream, addr) < 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "connect"), 0); 

        len = stream.send (buf, len+1); 
        if (len == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "send"), 0); 

        stream.close (); 
        return 0; 
    }

    void convert_to_unicode (ACE_FILE_Addr &io, ACE_FILE_Addr &addr) 
    {
        ACE_DEBUG ((LM_DEBUG, "convert to text.\n")); 
        convert_to_common (io, addr); 
    }
    
    void convert_to_mp3 (ACE_FILE_Addr &io, ACE_FILE_Addr &addr) 
    {
        ACE_DEBUG ((LM_DEBUG, "convert to mp3.\n")); 
        convert_to_common (io, addr); 
    }

    void convert_to_mpeg (ACE_FILE_Addr &io, ACE_FILE_Addr &addr) 
    {
        ACE_DEBUG ((LM_DEBUG, "convert to mpeg.\n")); 
        convert_to_common (io, addr); 
    }
}; 

class EncodeMessage : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        ACE_FILE_Addr addr = this->get_message_destination (msg); 
        switch (msg->type->value)
        {
        case 0: // text
            Util::convert_to_unicode (msg->io, addr); 
            break; 
        case 1: // audio
            Util::convert_to_mp3 (msg->io, addr); 
            break; 
        case 2:
            Util::convert_to_mpeg (msg->io, addr); 
            break; 
        default:
            ACE_ERROR_RETURN ((LM_ERROR, "invalid message type!!!\n"), -1); 
            break; 
        }

        msg->dest = addr; 
        return 0; 
    }

    ACE_FILE_Addr get_message_destination (Message *msg)
    {
        char path[MAXPATHLEN] = { 0 }; 
        strcpy (path, msg->io.get_path_name ()); 
        strcpy (path + strlen (path) - 3, "mp3"); 
        return ACE_FILE_Addr (path); 
    }
}; 

class SaveMetaData : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        msg->recorder->save_meta_data (msg->dest); 
        return 0; 
    }
}; 

class NotifySomeone : public BasicTask
{
public:
    virtual int process (Message *msg)
    {
        char buf[32] = { 0 }; 
        msg->caller_id->string (buf); 
        //ACE_DEBUG ((LM_INFO, "notify someone\n")); 
        ACE_DEBUG ((LM_INFO, 
            "New message from %s \n"
            "received and stored at %s\n", 
            buf, 
            msg->dest.get_path_name ())); 

        return 0; 
    }
}; 

class RecordingStream : public ACE_Stream <ACE_MT_SYNCH>
{
public: 
    typedef ACE_Stream <ACE_MT_SYNCH> BASE; 
    typedef ACE_Module <ACE_MT_SYNCH> MODULE; 

    RecordingStream () : BASE ()
    {
        // will call open
    }

    virtual int open (void *arg = 0,
        ACE_Module<ACE_MT_SYNCH> *head = 0,
        ACE_Module<ACE_MT_SYNCH> *tail = 0)
    {
        // we have been called in BASE constructor, 
        // here just replace the existing tail.
        MODULE *end_mod = 0; 
        ACE_NEW_RETURN (end_mod, MODULE ("end_module", new EndTask ()), -1); 
        if (this->replace ("ACE_Stream_Tail", end_mod) == -1)
            return -1; 

        //int ret = BASE::open (0, 0, end_mod); 
        //if (ret != 0)
        //    return ret; 

        //int n = 0;  
        MODULE *mod [8] = { 0 }; 
        // note here we can NOT use mod[n++]
        // as in the macro, it will n++ twice.
        ACE_NEW_RETURN (mod[0], MODULE ("answer_incoming_call", new AnswerIncomingCall ()), -1); 
        ACE_NEW_RETURN (mod[1], MODULE ("get_caller_id", new GetCallerId ()), -1); 
        ACE_NEW_RETURN (mod[2], MODULE ("play_outgoing_message", new PlayOutgoingMessage ()), -1); 
        ACE_NEW_RETURN (mod[3], MODULE ("record_incoming_message", new RecordIncomingMessage ()), -1); 
        ACE_NEW_RETURN (mod[4], MODULE ("release_device", new ReleaseDevice ()), -1); 
        ACE_NEW_RETURN (mod[5], MODULE ("encode_message", new EncodeMessage ()), -1); 
        ACE_NEW_RETURN (mod[6], MODULE ("save_meta_data", new SaveMetaData ()), -1); 
        ACE_NEW_RETURN (mod[7], MODULE ("notify_someone", new NotifySomeone ()), -1); 

        for (int i=8; i>0; -- i)
        {
            if (this->push (mod[i-1]) == -1)
                ACE_ERROR_RETURN ((LM_ERROR, "Failed to push module %d\n", i), -1); 
        }

        dump (); 
        return 0; 
    }

    int record (RecordingDevice *recorder)
    {
        ACE_Message_Block *mb = 0; 
        ACE_NEW_RETURN (mb, ACE_Message_Block (sizeof (Message)), -1); 

        Message *msg = (Message *)mb->rd_ptr (); 
        mb->wr_ptr (sizeof (Message)); 

        new (msg) Message (); 
        msg->recorder = recorder; 

        int ret = this->put (mb); 
        ACE_DEBUG ((LM_DEBUG, "put () returns %d\n", ret)); 
        return ret; 
    }
}; 

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    RecordingDevice *recorder = RecordingDeviceFactory::instantiate (argc, argv); 

    RecordingStream *recording_stream = 0; 
    ACE_NEW_RETURN (recording_stream, RecordingStream (), -1); 

    if (recording_stream->open () < 0)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "RecordingStream::open"), -1); 

    ACE_Time_Value future = ACE_OS::gettimeofday (); 
    future += 10; 

    //recorder.release (); 
    while (ACE_OS::gettimeofday () < future)
    {
        ACE_DEBUG ((LM_INFO, "Waiting for incoming message\n")); 
        RecordingDevice *activeRecorder = recorder->wait_for_activity (); 

        ACE_DEBUG ((LM_INFO, "Initiating recording process\n")); 
        recording_stream->record (activeRecorder); 
    }

    recording_stream->close (1); 
    delete recording_stream; 
    delete recorder; 
	return 0;
}

