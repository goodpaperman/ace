// hashmap.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ace/Hash_Map_With_Allocator_T.h" 
#include "ace/Malloc.h" 
#include "ace/Process.h" 
#include "ace/Process_Mutex.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/Auto_Ptr.h" 

#define BACKING_STORE "map.store" 
#define MAP_NAME "records.db" 
#define USE_WITH_ALLOCATOR

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
typedef ACE_Hash_Map_With_Allocator <int, Record> MAP; 
#else 
typedef ACE_Hash_Map_Manager <int, Record, ACE_Null_Mutex> MAP; 
#endif 

ACE_Process_Mutex mutex ("Coordinate-Mutex"); 

MAP *smap (ALLOCATOR *shmem)
{
    void *db = 0; 
    if (shmem->find (MAP_NAME, db) == 0)
        return (MAP *)db; 

    void *p = 0; 
    size_t tsize = sizeof (MAP); 
    ACE_ALLOCATOR_RETURN (p, 
        shmem->malloc (tsize), 
        0); 

    new (p) MAP (tsize, shmem); 
    if (shmem->bind (MAP_NAME, p) == -1)
    {
        ACE_ERROR ((LM_ERROR, "%p\n", "smap")); 
        //shmem->remove (); 
        return 0; 
    }

    return (MAP *)p; 
}

int add_records (MAP *map, ALLOCATOR *shmem)
{
    char buf[32] = { 0 }; 
    ACE_DEBUG ((LM_DEBUG, 
        "Map has %d entries, adding 20 more.\n", 
        map->current_size ())); 

    for (int i=0; i<20; ++ i)
    {
        ACE_OS::sprintf (buf, "%s: %d", "Record", i); 
        Record r (i, i+1, buf); 
        ACE_DEBUG ((LM_DEBUG, 
            "Adding a record for %d\n", 
            i)); 

        if (map->bind (i, r
#if defined (USE_WITH_ALLOCATOR)
            , shmem
#endif 
            ) == -1)
            ACE_ERROR_RETURN ((LM_ERROR, 
            "%p\n", "bind"), -1); 
    }

    return 0; 
}

int handle_parent (char *cmdline)
{
    ALLOCATOR *shmem = 0; 
    ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR, 
        ACE_MMAP_Memory_Pool_Options::ALWAYS_FIXED); 

    ACE_NEW_RETURN (shmem, ALLOCATOR (BACKING_STORE, 0, &options), -1); 
    MAP *map = smap (shmem); 
    if (map)
    {
        ACE_Process pa, pb; 
        ACE_Process_Options opt; 
        opt.command_line ("%s a", cmdline); 

        {
            ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
            ACE_DEBUG ((LM_DEBUG, "(%P|%t) Map has %d entries\n", map->current_size ())); 
            ACE_DEBUG ((LM_DEBUG, "(In parent, map is located at %@\n", map)); 
            pa.spawn (opt); 
            add_records (map, shmem); 
        }

        //ACE_OS::sleep (1); 
        pa.wait (); 

        {
            ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
            add_records (map, shmem); 
            ACE_DEBUG ((LM_DEBUG, "(%P|%t) Parent finished adding, map has %d entries.\n", map->current_size ())); 
            pb.spawn (opt); 
        }

        pb.wait (); 
    }

    shmem->remove (); 
    delete shmem; 
    return 0; 
}

int process_records (MAP *map, ALLOCATOR *shmem)
{
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Found %d records\n", map->current_size ())); 
    ACE_Auto_Array_Ptr <int> todelete (new int [map->current_size ()]); 

    int n = 0; 
    Record record; 
    for (MAP::iterator it = map->begin (); 
        it != map->end (); 
        ++ it)
    {
        ACE_DEBUG ((LM_DEBUG, 
            "(%P|%t) [%d] Preprocessing %d: %@\n", 
            n+1, 
            (*it).ext_id_, 
            &(*it).ext_id_)); 

        todelete [n++] = (*it).ext_id_; 
        int ret = map->find ((*it).ext_id_, record
#if defined (USE_WITH_ALLOCATOR)
            , shmem
#endif 
            ); 
        if (ret == -1)
            ACE_DEBUG ((LM_ERROR, 
            "Count not find record for %d\n", 
            (*it).ext_id_)); 
        else 
            ACE_DEBUG ((LM_DEBUG, 
            "Record name: %C|id1:%d|id2:%d\n", 
            record.name (), 
            record.id1 (), 
            record.id2 ())); 
    }

    for (int i=0; i<n; ++ i)
    {
        int ret = map->unbind (todelete [i]
#if defined (USE_WITH_ALLOCATOR)
        , shmem
#endif 
            ); 
        if (ret == -1)
            ACE_ERROR_RETURN ((LM_ERROR, 
            "Failed on key %d: %p\n", 
            "unbind", 
            todelete [i]), -1); 
        else 
            ACE_DEBUG ((LM_INFO, 
            "Fully processed and removed %d\n", 
            i)); 
    }

    return 0; 
}

int handle_child ()
{
#if defined (WIN32)
    // to stop the child process.
    DebugBreak (); 
#endif 

    ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, mutex, -1); 
    ALLOCATOR *shmem = 0; 
    ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR, 
        ACE_MMAP_Memory_Pool_Options::ALWAYS_FIXED); 

    ACE_NEW_RETURN (shmem, ALLOCATOR (BACKING_STORE, 0, &options), -1); 
    MAP *map = smap (shmem); 

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Map has %d entries\n", map->current_size ())); 
    ACE_DEBUG ((LM_DEBUG, "In child, map is located at %@\n", map)); 

    process_records (map, shmem); 
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

