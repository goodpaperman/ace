#include "StdAfx.h"
#include "HA_Compressor.h"

//ACE_FACTORY_DEFINE (ACE_Local_Service, HA_Compressor) 
//ACE_FACTORY_DEFINE (HA_Stream, HA_Compressor) 

extern "C" HA_Stream_Export void* _make_HA_Compressor (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Compressor(); 
}

ACE_STATIC_SVC_DEFINE (HA_Compressor,
                       "HA_Compressor", 
                       ACE_MODULE_T, 
                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Compressor, 
                       ACE_Service_Type::DELETE_OBJ |
                       ACE_Service_Type::DELETE_THIS, 
                       0) 

ACE_STATIC_SVC_REQUIRE (HA_Compressor) 

HA_Compressor::HA_Compressor(void)
: ACE_Module <ACE_MT_SYNCH> (
                             "HA_Compressor", 
                             new HA_Compressor_In (), 
                             new HA_Compressor_Out ())
{
}

HA_Compressor::~HA_Compressor(void)
{
}

int HA_Compressor_Out::svc () 
{
    char buf[4096] = { 0 }; 
    ACE_DEBUG ((LM_DEBUG, "HA_Compressor_Out startup\n")); 
    ACE_Message_Block *mb = 0, *mb_out = 0; 
    while (getq (mb) == 0)
    {
        mb_out = 0; 
        //ACE_DEBUG ((LM_DEBUG, "[compress] %s", mb->rd_ptr ())); 
        //ACE_LOG_MSG->log_hexdump (LM_INFO, mb->rd_ptr (), mb->length (), "[compress] "); 
        //ACE_OS::sprintf (buf, "!@#$%s", mb->rd_ptr ()); 
        compress (mb, mb_out); 
        mb->release (); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, mb_out->rd_ptr (), mb_out->length (), "[compress] "); 
        this->put_next (mb_out); 
        //mb->reset (); 
        //mb->copy (buf, strlen (buf) + 1); 
        //this->put_next (mb); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Compressor_Out shutdown\n")); 
    return 0; 
}

int HA_Compressor_Out::fini ()
{
    ACE_DEBUG ((LM_DEBUG, "HA_Compressor fini\n")); 
    return 0; 
}

int HA_Compressor_Out::compress (ACE_Message_Block *mb, ACE_Message_Block *&mb_out)
{
    int err;
    int mlen = mb->length (); //ACE_OS::strlen(mb->rd_ptr())+1;
    zstream_.next_in   = (Bytef*)mb->rd_ptr();
    zstream_.avail_in  = (uInt) mlen;
    zstream_.avail_out = (uInt) 5 * mb->size();

    mb_out = new ACE_Message_Block(zstream_.avail_out);
    zstream_.next_out  = (Bytef*)mb_out->wr_ptr();
    zstream_.zalloc = (alloc_func)0;
    zstream_.zfree = (free_func)0;
    zstream_.opaque = (voidpf)0;

    err = deflateInit_(&zstream_, Z_BEST_SPEED,
        ZLIB_VERSION, sizeof(zstream_));
    if (err != Z_OK)
    {
        return -1;
    }

    err = deflate(&zstream_, Z_FINISH);
    deflateEnd(&zstream_); 
    if (err != Z_STREAM_END)
    {
        return (err == Z_OK) ? Z_BUF_ERROR : err;
    }

    mb_out->wr_ptr(zstream_.total_out);
    //ACE_DEBUG ((LM_INFO, 
    //    "next_in = 0x%x\n"
    //    "next_out = 0x%x\n"
    //    "avail_in = %d\n"
    //    "avail_out = %d\n"
    //    "total_out = %d\n", 
    //    zstream_.next_in, 
    //    zstream_.next_out, 
    //    zstream_.avail_in, 
    //    zstream_.avail_out, 
    //    zstream_.total_out)); 
    return 0; 
}


int HA_Compressor_In::svc () 
{
    ACE_DEBUG ((LM_DEBUG, "HA_Compressor_In startup\n")); 
    ACE_Message_Block *mb = 0, *mb_out = 0; 
    while (getq (mb) == 0)
    {
        mb_out = 0; 
        //ACE_LOG_MSG->log_hexdump (LM_INFO, mb->rd_ptr (), mb->length (), "before inflate "); 
        uncompress (mb, mb_out); 
        mb->release (); 
        //ACE_DEBUG ((LM_DEBUG, "[uncompress] %s", mb_out->rd_ptr ())); 
        ACE_LOG_MSG->log_hexdump (LM_INFO, mb_out->rd_ptr (), mb_out->length (), "[uncompress] "); 
        this->put_next (mb_out); 
        //mb->rd_ptr (4); 
        //this->put_next (mb); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Compressor_In shutdown\n")); 
    return 0; 
}

int HA_Compressor_In::uncompress (ACE_Message_Block *mb, ACE_Message_Block *&mb_out)
{
    //ACE_DEBUG ((LM_INFO, "(%P|%t) Compress::recv() decompressing.\n" ));

    // Room for the decompressed data.  In the real world you
    // would probably want to send the original (uncompressed)
    // data size in the message.  You can predict the maximum
    // possible decompression size but it's cheap and easy 
    // just to send that along.  Look again at how I do
    // exacly that between Xmit and Recv.

    int err;

    zstream_.avail_out = 5 * mb->size();
    mb_out = new ACE_Message_Block(zstream_.avail_out);

    zstream_.next_in = (Bytef*)mb->rd_ptr();
    zstream_.avail_in = mb->size();
    zstream_.next_out = (Bytef*)mb_out->wr_ptr();
    zstream_.zalloc = (alloc_func)0;
    zstream_.zfree = (free_func)0;

    err = inflateInit(&zstream_);
    if (err != Z_OK)
    {
        return -1;
    }

    //ACE_DEBUG ((LM_INFO, 
    //    "next_in = 0x%x\n"
    //    "next_out = 0x%x\n"
    //    "avail_in = %d\n"
    //    "avail_out = %d\n"
    //    "total_out = %d\n", 
    //    zstream_.next_in, 
    //    zstream_.next_out, 
    //    zstream_.avail_in, 
    //    zstream_.avail_out, 
    //    zstream_.total_out)); 

    err = inflate(&zstream_, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        inflateEnd(&zstream_);
        return -1;
    }

    mb_out->wr_ptr(zstream_.total_out);
    //ACE_DEBUG ((LM_INFO, 
    //    "after inflate (%d): \n"
    //    "next_in = 0x%x\n"
    //    "next_out = 0x%x\n"
    //    "avail_in = %d\n"
    //    "avail_out = %d\n"
    //    "total_out = %d\n"
    //    "%s\n", 
    //    zstream_.total_out, 
    //    zstream_.next_in, 
    //    zstream_.next_out, 
    //    zstream_.avail_in, 
    //    zstream_.avail_out, 
    //    zstream_.total_out, 
    //    decompressed->rd_ptr ())); 

    return 0;
}