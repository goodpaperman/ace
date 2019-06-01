// allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h" 
#include "ace/Unbounded_Queue.h" 
#include "ace/Hash_Map_Manager.h" 
#include "ace/Message_Block.h" 
#include "ace/Null_Mutex.h" 

#define NUM 5
//#define USE_STATIC 
//#define USE_CACHED
//#define USE_DYNAMIC_CACHED

#if defined (USE_CACHED)
#include "ace/Malloc_T.h" 
typedef ACE_Cached_Allocator <char[64], ACE_Null_Mutex> BASE_ALLOC; 
#elif defined (USE_DYNAMIC_CACHED)
#include "ace/Malloc_T.h" 
typedef ACE_Dynamic_Cached_Allocator <ACE_Null_Mutex> BASE_ALLOC; 
#elif defined (USE_STATIC)
#include "ace/Malloc_T.h" 
typedef ACE_Static_Allocator <2048> BASE_ALLOC; 
#else // the default one
#include "ace/Malloc_Allocator.h"
typedef ACE_New_Allocator BASE_ALLOC; 
#endif 

// an dumper.
class Local_Allocator : public BASE_ALLOC
{
public:
    Local_Allocator ()
#if defined (USE_CACHED)
        : BASE_ALLOC (1024) 
#elif defined (USE_DYNAMIC_CACHED)
        : BASE_ALLOC (100, 60)
#endif 
    {
    }
  virtual void *malloc (size_t nbytes)
  {
      void *p = BASE_ALLOC::malloc (nbytes); 
      ACE_DEBUG ((LM_DEBUG, "malloc %u bytes at %@.\n", nbytes, p)); 
      return p; 
  }

  virtual void *calloc (size_t nbytes, char initial_value = '\0')
  {
      ACE_DEBUG ((LM_DEBUG, "calloc %u bytes with value %u.\n", nbytes, initial_value)); 
      return BASE_ALLOC::calloc (nbytes, initial_value); 
  }

  virtual void *calloc (size_t n_elem, size_t elem_size, char initial_value = '\0')
  {
      ACE_DEBUG ((LM_DEBUG, "calloc %u elem with size %u and value %u.\n", n_elem, elem_size, initial_value)); 
      return BASE_ALLOC::calloc (n_elem, elem_size, initial_value); 
  }

  virtual void free (void *ptr)
  {
      ACE_DEBUG ((LM_DEBUG, "free ptr %@.\n", ptr)); 
      return BASE_ALLOC::free (ptr); 
  }
}; 

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    Local_Allocator alloc; 
    ACE_Message_Block *mb = 0; 

#if 1
    ACE_NEW_MALLOC_RETURN (mb, 
        (ACE_Message_Block *)alloc.malloc (sizeof ACE_Message_Block), 
        ACE_Message_Block (64, 
                           ACE_Message_Block::MB_DATA, 
                           0, 
                           0, 
                           &alloc, 
                           0, 
                           0, 
                           ACE_Time_Value::zero, 
                           ACE_Time_Value::max_time, 
                           &alloc, 
                           &alloc), 
        -1); 

    if (mb)
        mb->release (); 
#endif 

#if 1
    ACE_Unbounded_Queue <ACE_Message_Block *> queue (&alloc); 
    for (int i=0; i<NUM; ++ i)
        queue.enqueue_tail (new ACE_Message_Block (64, 
                                                   ACE_Message_Block::MB_DATA, 
                                                   0, 
                                                   0, 
                                                   &alloc, 
                                                   0, 
                                                   0, 
                                                   ACE_Time_Value::zero, 
                                                   ACE_Time_Value::max_time, 
                                                   &alloc, 
                                                   0/*&alloc*/)); 

    while (!queue.is_empty ())
    {
        queue.dequeue_head (mb); 
        mb->release (); 
    }
#endif 

#if 1
    ACE_Hash_Map_Manager <int, ACE_Message_Block *, ACE_Null_Mutex> map (NUM-1, &alloc); 
    for (int i=0; i<NUM; ++ i)
        map.bind (i, 
                    new ACE_Message_Block (64, 
                                           ACE_Message_Block::MB_DATA, 
                                           0, 
                                           0, 
                                           &alloc, 
                                           0, 
                                           0, 
                                           ACE_Time_Value::zero, 
                                           ACE_Time_Value::max_time, 
                                           &alloc, 
                                           0/*&alloc*/)); 

    for (int i=0; i<NUM; ++ i)
    {
        if (map.unbind (i, mb) == 0)
            mb->release (); 
    }
#endif 

	return 0;
}

