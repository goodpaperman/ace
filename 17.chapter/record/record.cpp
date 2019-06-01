// record.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/OS_NS_stdio.h" 
#include "ace/Null_Mutex.h" 
#include "ace/Malloc.h" 
#include "ace/PI_Malloc.h" 

#define USE_PI

#if defined (USE_PI)
typedef ACE_Malloc_T <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex, ACE_PI_Control_Block> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex, ACE_PI_Control_Block> ITERATOR; 
#else
typedef ACE_Malloc <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex> ITERATOR; 
#endif 

typedef ACE_MMAP_Memory_Pool_Options OPTIONS; 

#define BACKING_STORE "backing.store" 
ALLOCATOR *g_allocator = 0; 

class Record 
{
public:
    Record (int id1, int id2, char *name)
        : id1_ (id1), id2_ (id2), name_ (0)
    {
        size_t len = ACE_OS::strlen (name) + 1; 
        char *buf = (char *)g_allocator->malloc (len); 
        ACE_OS::strcpy (buf, name); 
        name_ = buf; 
    }

    ~ Record () { g_allocator->free (name_); } 

    char* name () { return name_; } 
    int id1 () { return id1_; } 
    int id2 () { return id2_; } 

private:
    int id1_, id2_; 
#if defined (USE_PI)
    ACE_Based_Pointer_Basic <char> name_; 
#else 
    char *name_; 
#endif 
}; 

int add_records ()
{
    char buf[32] = { 0 }; 
    for (int i=0; i<10; ++ i)
    {
        ACE_OS::sprintf (buf, "%s:%d", "Record", i); 
        void *p = g_allocator->malloc (sizeof (Record)); 
        if (p == 0)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "Unable to malloc"), -1); 

        Record *r = new (p) Record (i, i+1, buf); 
        if (g_allocator->bind (buf, r) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "bind failed"), -1); 
    }

    return 0; 
}

int show_records ()
{
    ACE_DEBUG ((LM_DEBUG, "The following records were found:\n")); 

    ITERATOR iter (*g_allocator); 
    for (void *p=0; iter.next (p) != 0; iter.advance ())
    {
        Record *r = (Record *)p; 
        ACE_DEBUG ((LM_DEBUG, "Record name: %C | id1: %d | id2: %d\n", 
            r->name (), 
            r->id1 (), 
            r->id2 ())); 
    }

    return 0; 
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
#if defined (USE_PI)
    // position independent 
    // don't need to load in same base address.
    ACE_NEW_RETURN (g_allocator, ALLOCATOR (BACKING_STORE), -1); 
#else 
    OPTIONS options; 
    ACE_NEW_RETURN (g_allocator, ALLOCATOR (BACKING_STORE, 0, &options), -1); 
#endif 

    ACE_DEBUG ((LM_DEBUG, "base address = %@\n", g_allocator->base_addr ())); 
    if (argc > 1)
        show_records (); 
    else 
        add_records (); 

    g_allocator->sync (); 
    delete g_allocator; 
	return 0;
}

