// malloc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/PI_Malloc.h" 
#include "ace/Malloc_T.h" 
#include "ace/Null_Mutex.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/Process_Mutex.h" 
#include <vector> 
#include <algorithm> 

using std::vector; 


#define NUM 50
#define ALLOC_SIZE 500       //2032
#define ALLOCATOR_TYPE 4
#define USE_PI

//#define LOCK_TYPE ACE_Null_Mutex
//#define LOCK_TYPE ACE_Thread_Mutex 
#define LOCK_TYPE ACE_Process_Mutex 

#if defined (USE_PI)
#define CB_TYPE ACE_PI_Control_Block
#else 
#define CB_TYPE ACE_Control_Block 
#endif 

#if !defined (ALLOCATOR_TYPE) || (ALLOCATOR_TYPE == 1)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_LOCAL_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR;  
typedef ACE_Malloc_LIFO_Iterator_T <ACE_LOCAL_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_Local_Memory_Pool_Options OPTION; 
#  else 
typedef ACE_Malloc <ACE_LOCAL_MEMORY_POOL, LOCK_TYPE> ALLOCATOR;  
typedef ACE_Malloc_LIFO_Iterator <ACE_LOCAL_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_Local_Memory_Pool_Options OPTION; 
#  endif 
#elif (ALLOCATOR_TYPE == 2)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_MMAP_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_MMAP_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_MMAP_Memory_Pool_Options OPTION; 
#  else 
typedef ACE_Malloc <ACE_MMAP_MEMORY_POOL, LOCK_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_MMAP_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_MMAP_Memory_Pool_Options OPTION; 
#  endif 
#elif (ALLOCATOR_TYPE == 3)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_LITE_MMAP_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_LITE_MMAP_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_MMAP_Memory_Pool_Options OPTION; 
#  else 
typedef ACE_Malloc <ACE_LITE_MMAP_MEMORY_POOL, LOCK_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_LITE_MMAP_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_MMAP_Memory_Pool_Options OPTION; 
#  endif 
#elif (ALLOCATOR_TYPE == 4)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_PAGEFILE_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_PAGEFILE_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_Pagefile_Memory_Pool_Options OPTION; 
#  else
typedef ACE_Malloc <ACE_PAGEFILE_MEMORY_POOL, LOCK_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_PAGEFILE_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_Pagefile_Memory_Pool_Options OPTION; 
#  endif 
#elif (ALLOCATOR_TYPE == 5)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_SBRK_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_SBRK_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_Sbrk_Memory_Pool_Options OPTION; 
#  else
typedef ACE_Malloc <ACE_SBRK_MEMORY_POOL, LOCK_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_SBRK_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_Sbrk_Memory_Pool_Options OPTION; 
#  endif 
#elif (ALLOCATOR_TYPE == 6)
#  if defined (USE_PI)
typedef ACE_Malloc_T <ACE_SHARED_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator_T <ACE_SHARED_MEMORY_POOL, LOCK_TYPE, CB_TYPE> ITERATOR; 
typedef ACE_Shared_Memory_Pool_Options OPTION; 
#  else
typedef ACE_Malloc <ACE_SHARED_MEMORY_POOL, LOCK_TYPE> ALLOCATOR; 
typedef ACE_Malloc_LIFO_Iterator <ACE_SHARED_MEMORY_POOL, LOCK_TYPE> ITERATOR; 
typedef ACE_Shared_Memory_Pool_Options OPTION; 
#  endif 
#else 
#error invalid ALLOCATOR_TYPE value.
#endif 

//typedef ACE_Malloc <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex> ALLOCATOR; 


void print_alignment_info ()
{
    ACE_DEBUG ((LM_DEBUG, ACE_LIB_TEXT ("Start ---> print_alignment_info:\n")));
    ACE_DEBUG ((LM_DEBUG,
        ACE_LIB_TEXT ("Sizeof ptr: %d\n")
        ACE_LIB_TEXT ("Sizeof size_t: %d\n")
        ACE_LIB_TEXT ("Sizeof long: %d\n")
        ACE_LIB_TEXT ("Sizeof double: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_MALLOC_ALIGN: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_MALLOC_PADDING: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_MALLOC_HEADER_SIZE: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_MALLOC_PADDING_SIZE: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_CONTROL_BLOCK_SIZE: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_CONTROL_BLOCK_ALIGN_BYTES: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_PI_MALLOC_PADDING_SIZE: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_PI_CONTROL_BLOCK_SIZE: %d\n")
        ACE_LIB_TEXT ("Sizeof ACE_PI_CONTROL_BLOCK_ALIGN_BYTES: %d\n")
        ACE_LIB_TEXT ("Sizeof (MALLOC_HEADER): %d\n")
        ACE_LIB_TEXT ("Sizeof (CONTROL_BLOCK): %d\n")
        ACE_LIB_TEXT ("Sizeof (PI_MALLOC_HEADER): %d\n")
        ACE_LIB_TEXT ("Sizeof (PI_CONTROL_BLOCK): %d\n"),
        sizeof (char *),
        sizeof (size_t),
        sizeof (long),
        sizeof (double),
        ACE_MALLOC_ALIGN, 
        ACE_MALLOC_PADDING,
        ACE_MALLOC_HEADER_SIZE,
        ACE_MALLOC_PADDING_SIZE, 
        ACE_CONTROL_BLOCK_SIZE,
        ACE_CONTROL_BLOCK_ALIGN_BYTES,
        ACE_PI_MALLOC_PADDING_SIZE,
        ACE_PI_CONTROL_BLOCK_SIZE,
        ACE_PI_CONTROL_BLOCK_ALIGN_BYTES,
        sizeof (ACE_Control_Block::ACE_Malloc_Header), 
        sizeof (ACE_Control_Block), 
        sizeof (ACE_PI_Control_Block::ACE_Malloc_Header),
        sizeof (ACE_PI_Control_Block)
        ));
    ACE_DEBUG ((LM_DEBUG, ACE_LIB_TEXT ("End <--- print_alignment_info:\n")));
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    ACE_OS::srand (ACE_OS::getpid ()); 
    print_alignment_info (); 

#if !defined (ALLOCATOR_TYPE)
    OPTION option; 
//#elif (ALLOCATOR_TYPE == 2)
//    // this is the default.
//    OPTION option ((void *)0x4000000, OPTION::ALWAYS_FIXED); 
//    //OPTION option ((void *)0x2000000, OPTION::ALWAYS_FIXED); 
#elif ((ALLOCATOR_TYPE == 2) || (ALLOCATOR_TYPE == 3)) && defined (USE_PI)
    OPTION option ((void *)0);  // FIRSTCALL_FIXED
#elif (ALLOCATOR_TYPE == 4) && !defined (USE_PI)
    OPTION option ((void *)0x2000000, ACE_DEFAULT_PAGEFILE_POOL_SIZE); 
#else 
    OPTION option; 
#endif 

    ALLOCATOR m ("c:/file.map", "non-lock", &option); 
    char* name[NUM] = { 0 }; 
    m.print_stats (); 

    int size = 0; 
    vector <int> pos; 
    for (int i=0; i<NUM; ++ i)
    {
        pos.push_back (i); 
        size = ALLOC_SIZE * (ACE_OS::rand () % 10 + 1); 
        if (i > NUM / 2)
            size = 100; 

        //name[i] = (char *)m.malloc (100); 
        name[i] = (char *)m.calloc (size); 
        if (name[i])
        {
            //ACE_OS::sprintf (name[i], "this is string %u.\n", i); 
            ACE_OS::snprintf (name[i], ALLOC_SIZE, "this is string %u.\n", i); 
            ACE_DEBUG ((LM_DEBUG, "alloc %u bytes at %@\n", size, name[i])); 

            if (m.bind (name[i], name[i], 1) != 0)
            {
                ACE_DEBUG ((LM_DEBUG, "%p\n", "bind failed")); 
                m.free (name[i]); 
                name[i] = 0; 
            }

            m.print_stats (); 
            //ACE_OS::sleep (1); 
        }
        else 
        {
            ACE_DEBUG ((LM_DEBUG, "%p\n", "allocate error")); 
            break; 
        }
    }

    //m.print_stats (); 
    m.sync (); 

    void *p = 0; 
    if (m.find ("this is string 25.\n", p) == 0)
    {
        ACE_DEBUG ((LM_DEBUG, "find at %@: %s\n", p, p)); 
        ACE_LOG_MSG->log_hexdump (LM_DEBUG, (char *)p-100, 4096); 
    }

    {
        // iterator will hold the allocator lock
        // untill destroyed.
        void *pointer = 0; 
        char const* key = 0; 
        ITERATOR it (m); 
        for (; !it.done (); it.advance ())
        {
            it.next (pointer, key); 
            ACE_DEBUG ((LM_DEBUG, "%@: %s", pointer, key)); 
        }
    }

    ACE_OS::sleep (1); 

    std::random_shuffle (pos.begin(), pos.end()); 
    for (int i=0; i<pos.size(); ++ i)
    {
        if (name[pos[i]])
        {
            m.unbind (name[pos[i]], p); 
            ACE_DEBUG ((LM_DEBUG, "unbind at %@: %s\n", p, p)); 
            m.free (name[pos[i]]); 
            m.print_stats (); 
        }
        //ACE_OS::sleep (1); 
    }

    //m.print_stats (); 

    // not remove.
    //m.release (1); 

    // should remove backstore file, 
    // to avoid next startup crash, 
    // as we will map it for different base address each time.
    //m.release (0); 
    //m.remove (); 
	return 0;
}

