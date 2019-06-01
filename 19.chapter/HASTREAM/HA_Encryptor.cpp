#include "StdAfx.h"
#include "HA_Encryptor.h"
#include "ace/Get_Opt.h" 
#include <xutility> 

//ACE_FACTORY_DEFINE (ACE_Local_Service, HA_Encryptor) 
//ACE_FACTORY_DEFINE (HA_Stream, HA_Encryptor) 

extern "C" HA_Stream_Export void* _make_HA_Encryptor (ACE_Service_Object_Exterminator *gobbler)
{
    if (gobbler)
        *gobbler = 0; 

    return new HA_Encryptor (); 
}

//ACE_STATIC_SVC_DEFINE (HA_Encryptor,
//                       "HA_Encryptor", 
//                       ACE_MODULE_T, 
//                       (ACE_SERVICE_ALLOCATOR)&_make_HA_Encryptor, 
//                       ACE_Service_Type::DELETE_OBJ |
//                       ACE_Service_Type::DELETE_THIS, 
//                       0) 
//
//ACE_STATIC_SVC_REQUIRE (HA_Encryptor) 

HA_Encryptor::HA_Encryptor(void)
: ACE_Module <ACE_MT_SYNCH> (
                             "HA_Encryptor", 
                             new HA_Encryptor_In (), 
                             new HA_Encryptor_Out (), 
                             (void *)&data_)
, data_ ()
{
}

HA_Encryptor::~HA_Encryptor(void)
{
}

int HA_Encryptor_Out::init (int argc, ACE_TCHAR *argv[])
{
    EncryptionData *data = (EncryptionData *)module ()->arg (); 
    static const ACE_TCHAR options [] = ACE_TEXT (":c"); 
    ACE_Get_Opt opts (argc, argv, options, 0); 

    data->is_client = false; 
    int option = 0; 
    while ((option = opts ()) != EOF)
    {
        switch (option)
        {
        case 'c':
            // to be a client.
            data->is_client = true; 
            break; 
        case ':':
            ACE_ERROR_RETURN ((LM_ERROR, "-%c requires an argument\n", opts.opt_opt ()), -1); 
        default:
            ACE_ERROR_RETURN ((LM_ERROR, "Parse error\n"), -1); 
        }
    }

    return 0; 
}

int HA_Encryptor_Out::fini ()
{
    ACE_DEBUG ((LM_DEBUG, "HA_Encryptor fini\n")); 
    EncryptionData *data = (EncryptionData *)module ()->arg (); 
    RSA_free (data->peer_public_key); 
    RSA_free (data->my_public_key); 
    RSA_free (data->my_private_key); 
    data->peer_public_key = 0; 
    data->my_public_key = 0; 
    data->my_private_key = 0; 
    return 0; 
}

int HA_Encryptor_Out::svc () 
{
    ACE_DEBUG ((LM_DEBUG, "HA_Encryptor_Out startup\n")); 
    EncryptionData *data = (EncryptionData *)module ()->arg (); 

    // first generate keys whether we are server or client.
    RSA* key_pair = RSA_generate_key (2048, 65537, NULL, NULL);

    // 校验密钥对
    if(RSA_check_key(key_pair) != 1)
    {
        ACE_DEBUG ((LM_DEBUG, "RSA_check_key Error\n"));
        return -1;
    }

    // 提取公钥
    unsigned char *p = 0; 
    const unsigned char* cp=0; 

    // 提取私钥
    p = data->buf;
    data->len = i2d_RSAPrivateKey(key_pair, &p);
    ACE_LOG_MSG->log_hexdump (LM_INFO, (char *)data->buf, data->len, "[private key] "); 

    cp = data->buf;
    data->my_private_key = d2i_RSAPrivateKey (NULL, &cp, data->len);

    p = data->buf;
    data->len = i2d_RSAPublicKey(key_pair, &p);
    ACE_LOG_MSG->log_hexdump (LM_INFO, (char *)data->buf, data->len, "[public key] "); 

    cp = data->buf; 
    data->my_public_key = d2i_RSAPublicKey (NULL, &cp, data->len);

    RSA_free(key_pair);

    if (data->is_client)
    {
        // issue the request to exchange the key.
        // wait Xmit to establish the connection
        ACE_OS::sleep (2); 
        ACE_Message_Block *mb = new ACE_Message_Block (data->len); 
        mb->copy ((char *)data->buf, data->len); 
        this->put_next (mb); 
    }

    ACE_Message_Block *mb = 0; 
    while (getq (mb) == 0)
    {
        //ACE_DEBUG ((LM_DEBUG, "[encrypt] %s", mb->rd_ptr ())); 
        // skip '\n' & '\0'
        //for (int i=0; i<mb->length ()-2; ++ i)
        //    mb->rd_ptr () [i] += 4; 

        ACE_Message_Block *mblk = new ACE_Message_Block (4096); 
        int len = encrypt ((byte *)mb->rd_ptr (), mb->length (), (byte *)mblk->rd_ptr ()); 
        mblk->wr_ptr (len); 
        mb->release (); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, mblk->rd_ptr (), mblk->length (), "[encrypt] "); 
        this->put_next (mblk); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Encryptor_Out shutdown\n")); 
    return 0; 
}


int HA_Encryptor_Out::encrypt(byte *in, unsigned len, byte *out)
{
    EncryptionData *data = (EncryptionData *)module ()->arg (); 
    RSA *key = data->peer_public_key;

    int ret = 0;
    const unsigned block_size = RSA_size(key) - PKI_RSA_PKCS1_OAEP_OVERHEAD - 1;
    while(len > 0)
    {
        unsigned size = min (len, block_size);

        /* Perform the encryption */
        int rv = RSA_public_encrypt(size, in, out, key, RSA_PKCS1_OAEP_PADDING);

        /* Advance pointers and adjust counters */
        out += rv;
        ret += rv;

        in += size;
        len -= size;
    }

    return ret;
}

int HA_Encryptor_In::svc () 
{
    ACE_DEBUG ((LM_DEBUG, "HA_Encryptor_In startup\n")); 
    EncryptionData *data = (EncryptionData *)module ()->arg (); 

    ACE_Message_Block *mb = 0; 
    while (getq (mb) == 0)
    {
        if (data->key_exchanged == 0)
        {
            // this is a response packet replyed from server.
            // let client process this flag.
            data->key_exchanged = 1; 

            int len = mb->length (); 
            unsigned char buf[MAX_KEY_LEN] = { 0 }, *p = 0; 
            ACE_OS::memcpy ((char *)buf, mb->rd_ptr (), len); 
            mb->release (); 

            const unsigned char* cp=buf; 
            data->peer_public_key = d2i_RSAPublicKey (NULL, &cp, len);
            ACE_LOG_MSG->log_hexdump (LM_INFO, (char *)buf, len, "[peer public key] "); 
            
            if (data->is_client == 0)
            {
                // only reply if we are the server.
                mb = new ACE_Message_Block (data->len); 
                mb->copy ((char *)data->buf, data->len); 
                this->reply (mb); 
            }

            continue; 
        }

        //ACE_DEBUG ((LM_DEBUG, "[unencrypt] %s", mb->rd_ptr ())); 
        // skip '\n' & '\0'
        //for (int i=0; i<mb->length ()-2; ++ i)
        //    mb->rd_ptr () [i] -= 4; 

        ACE_Message_Block *mblk = new ACE_Message_Block (4096); 
        int len = decrypt ((byte *)mb->rd_ptr (), mb->length (), (byte *)mblk->rd_ptr ()); 
        mblk->wr_ptr (len); 
        mb->release (); 

        ACE_LOG_MSG->log_hexdump (LM_INFO, mblk->rd_ptr (), mblk->length (), "[unencrypt] "); 
        this->put_next (mblk); 
    }

    ACE_DEBUG ((LM_DEBUG, "HA_Encryptor_In shutdown\n")); 
    return 0; 
}


int HA_Encryptor_In::decrypt(byte *in, unsigned len,  byte *out)
{
    EncryptionData *data = (EncryptionData *)module ()->arg (); 
    RSA *key = data->my_private_key;

    int ret = 0; 
    const unsigned block_size = RSA_size(key);
    while(len > 0)
    {
        unsigned size = min (len, block_size);

        /* Perform the encryption */
        int rv = RSA_private_decrypt(size, in, out, key, RSA_PKCS1_OAEP_PADDING);

        /* Advance pointers and adjust counters */
        out += rv;
        ret += rv;

        in += size;
        len -= size;
    }

    return ret;
}