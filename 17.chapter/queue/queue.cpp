// queue.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Unbounded_Queue.h" 
//#include "ace/Hash_Map_With_Allocator_T.h" 
#include "ace/Malloc.h" 
#include "ace/Process.h" 
#include "ace/Process_Mutex.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/Auto_Ptr.h" 

#define BACKING_STORE "que.store" 
#define QUEUE_NAME "records.db" 
#define USE_WITH_ALLOCATOR

template <typename T>
class ACE_Unbounded_Queue_With_Allocator : public ACE_Unbounded_Queue <T> 
{
public:
    typedef ACE_Unbounded_Queue <T> BASE; 
    ACE_Unbounded_Queue_With_Allocator (ACE_Allocator *alloc) 
        : BASE (alloc)
    {
    }

  int enqueue_tail (const T &new_item, ACE_Allocator *alloc)
  {
      BASE::allocator_ = alloc; 
      return BASE::enqueue_tail (new_item); 
  }

  int enqueue_head (const T &new_item, ACE_Allocator *alloc)
  {
      BASE::allocator_ = alloc; 
      return BASE::enqueue_head (new_item); 
  }

  int dequeue_head (T &item, ACE_Allocator *alloc)
  {
      BASE::allocator_ = alloc; 
      return BASE::dequeue_head (item); 
  }
}; 

class Record 
{
public:
    Record () { }
    ~ Record () { }

    Record (Record const& rhs) 
        : id1_ (rhs.id1_)
        , id2_ (rhs.id2_) 
    {
        ACE_OS::strcpy (name_, rhs.name_); 
    }

    Record (int id1, int id2, char const* name)
        : id1_ (id1)
        , id2_ (id2) 
    {
        ACE_OS::strcpy (name_, name); 
    }

    char const* name () const { return name_; } 
    int id1 () const { return id1_; } 
    int id2 () const { return id2_; } 

private:
    int id1_; 
    int id2_; 
    char name_[128]; 
}; 


typedef ACE_Malloc <ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex> MALLOC; 
typedef ACE_Allocator_Adapter <MALLOC> ALLOCATOR; 
#if defined (USE_WITH_ALLOCATOR)
typedef ACE_Unbounded_Queue_With_Allocator <Record> QUEUE; 
#else 
typedef ACE_Unbounded_Queue <Record> QUEUE; 
#endif 

ACE_Process_Mutex mutex ("Coordinate-Mutex"); 
ALLOCATOR *g_shmem = 0; 

QUEUE *squeue (ALLOCATOR *shmem)
{
    ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, 0); 
    void *db = 0; 
    if (shmem->find (QUEUE_NAME, db) == 0)
        return (QUEUE *)db; 

    void *p = 0; 
    size_t tsize = sizeof (QUEUE); 
    ACE_ALLOCATOR_RETURN (p, 
        shmem->malloc (tsize), 
        0); 

    new (p) QUEUE (shmem); 
    if (shmem->bind (QUEUE_NAME, p) == -1)
    {
        ACE_ERROR ((LM_ERROR, "%p\n", "squeue")); 
        //shmem->remove (); 
        return 0; 
    }

    return (QUEUE *)p; 
}

int send_record (int i, QUEUE *que, ALLOCATOR *shmem)
{
    ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
    char buf[32] = { 0 }; 
    ACE_OS::sprintf (buf, "%s: %d", "Record", i); 
    Record r (i, i+1, buf); 
    ACE_DEBUG ((LM_DEBUG, 
        "Adding a record for %d\n", 
        i)); 

    if (que->enqueue_tail (r
#if defined (USE_WITH_ALLOCATOR)
        , shmem
#endif 
        ) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, 
        "%p\n", "enqueue_tail"), -1); 

    return 0; 
}

int handle_parent (char *cmdline)
{
    ALLOCATOR *shmem = 0; 
    ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR, 
        ACE_MMAP_Memory_Pool_Options::ALWAYS_FIXED); 

    ACE_NEW_RETURN (shmem, ALLOCATOR (BACKING_STORE, 0, &options), -1); 
    QUEUE *que = squeue (shmem); 

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Queue has %d entries\n", que->size ())); 
    ACE_DEBUG ((LM_DEBUG, "In parent, que is located at %@\n", que)); 

    ACE_Process pa, pb; 
    ACE_Process_Options opt; 
    opt.command_line ("%s a", cmdline); 

    pa.spawn (opt); 
    pb.spawn (opt); 

    for (int i=0; i<100; ++ i)
        send_record (i, que, shmem); 

    send_record (-1, que, shmem); 

    pa.wait (); 
    pb.wait (); 

    shmem->remove (); 
    delete shmem; 
    return 0; 
}

int process_record (QUEUE *que, ALLOCATOR *shmem)
{
    ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
    if (que->is_empty ())
    {
        ace_mon.release (); 
        ACE_OS::sleep (ACE_Time_Value (0, 1000)); 
        return 0; 
    }

    Record record; 
    if (que->dequeue_head (record
#if defined (USE_WITH_ALLOCATOR)
        , shmem
#endif 
        ) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "dequeue_head"), -1); 

    ACE_DEBUG ((LM_DEBUG, 
        "(%P|%t) Processing record | name: %C"
        " | id1: %d | id2: %d\n", 
        record.name (), 
        record.id1 (), 
        record.id2 ())); 

    if (record.id1 () == -1)
        que->enqueue_tail (record
#if defined (USE_WITH_ALLOCATOR)
        , shmem
#endif 
        ); 

    return record.id1 (); 
}

#if defined (WIN32)
int handle_remap (EXCEPTION_POINTERS *ep)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Handle a remap\n")); 
    DWORD ecode = ep->ExceptionRecord->ExceptionCode; 
    if (ecode != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH; 

    void *addr = (void *)ep->ExceptionRecord->ExceptionInformation[1]; 
    if (g_shmem->alloc ().memory_pool ().remap (addr) == -1)
        return EXCEPTION_CONTINUE_SEARCH; 

#if __X86__
    ep->ContextRecord->Edi = (DWORD)addr; 
#elif __MIPS__
    ep->ContextRecord->IntA0 = ep->ContextRecord->IntV0 = (DWORD) addr; 
    ep->ContextRecord->IntT5 = ep->ContextRecord->IntA0 + 3; 
#endif 

    return EXCEPTION_CONTINUE_EXECUTION; 
}

int process_record_win32 (QUEUE *que, ALLOCATOR *shmem)
{
    ACE_SEH_TRY
    {
        return process_record (que, shmem); 
    }
    ACE_SEH_EXCEPT (handle_remap (GetExceptionInformation ()))
    {
    }

    return 0; 
}
#endif 

int handle_child ()
{
    //ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
    ALLOCATOR *shmem = 0; 
    ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR, 
        ACE_MMAP_Memory_Pool_Options::ALWAYS_FIXED); 

    ACE_NEW_RETURN (shmem, ALLOCATOR (BACKING_STORE, 0, &options), -1); 
    g_shmem = shmem; 
    QUEUE *que = squeue (shmem); 
    if (que == 0)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "could not obtain queue"), -1); 

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Queue has %d entries\n", que->size ())); 
    ACE_DEBUG ((LM_DEBUG, "In child, que is located at %@\n", que)); 

    ACE_OS::sleep (2); 
#if defined (WIN32)
    // to stop the child process.
    //DebugBreak (); 
#endif 

#if defined (WIN32)
    while (process_record_win32 (que, shmem) != -1)
#else 
    while (process_record (que, shmem) != -1)
#endif 
        ; 

    shmem->sync (); 
    delete shmem; 
    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    if (argc == 1)
        ACE_ASSERT (handle_parent (argv[0]) == 0); 
    else 
        ACE_ASSERT (handle_child () == 0); 

	return 0;
}

