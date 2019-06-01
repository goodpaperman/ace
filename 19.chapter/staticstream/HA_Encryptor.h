#pragma once

#include "ace/Module.h" 
#include "HA_Task.h" 
#include "openssl/rsa.h" 

typedef unsigned char uchar;
typedef unsigned char byte;
#define PKI_PUBLIC_KEY_LENGTH           280
#define PKI_PRIVATE_KEY_LENGTH          1200
#define PKI_MAX_KEY_LENGTH              ((PKI_PUBLIC_KEY_LENGTH > PKI_PRIVATE_KEY_LENGTH ? PKI_PUBLIC_KEY_LENGTH : PKI_PRIVATE_KEY_LENGTH) + 4)
#define MAX_KEY_LEN                     PKI_MAX_KEY_LENGTH
#define PKI_RSA_PKCS1_OAEP_OVERHEAD     41
#define PKI_SIGNATURE_LENGTH            256

struct EncryptionData
{
    int is_client; 
    int key_exchanged; 
    RSA *my_public_key; 
    RSA *my_private_key; 
    RSA *peer_public_key; 
    // really public key len & buffer.
    int len; 
    unsigned char buf[MAX_KEY_LEN]; 
}; 

class HA_Encryptor_Out : public HA_Task
{
public:
    virtual int init (int argc, ACE_TCHAR *argv[]);
    virtual int fini (); 
    virtual int svc (); 
    int encrypt(byte *in, unsigned len, byte *out); 
}; 

class HA_Encryptor_In : public HA_Task
{
public:
    virtual int svc (); 
    int decrypt(byte *in, unsigned len,  byte *out); 
}; 

class HA_Encryptor : public ACE_Module <ACE_MT_SYNCH>/*, public ACE_Service_Object */
{
public:
    HA_Encryptor(void);
    ~HA_Encryptor(void);

private: 
    EncryptionData data_; 
};

//ACE_FACTORY_DECLARE (ACE_Local_Service, HA_Encryptor)
//ACE_FACTORY_DECLARE (HA_Stream, HA_Encryptor)