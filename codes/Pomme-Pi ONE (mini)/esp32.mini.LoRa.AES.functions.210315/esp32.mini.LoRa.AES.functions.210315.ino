#include "mbedtls/aes.h"

void encrypt(unsigned char *plainText,char *key,unsigned char *outputBuffer, int nblocks)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_enc(&aes,(const unsigned char*)key,strlen(key)*8);
  for(int i=0;i<nblocks;i++)
    {
    mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_ENCRYPT,
                        (const unsigned char*)(plainText+i*16), outputBuffer+i*16);
    }                 
  mbedtls_aes_free(&aes);
}

void decrypt(unsigned char *chipherText,char *key,unsigned char *outputBuffer, int nblocks)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
    for(int i=0;i<nblocks;i++)
    {
    mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_DECRYPT,
                       (const unsigned char*)(chipherText+i*16), outputBuffer+i*16);
    }                   
  mbedtls_aes_free(&aes );
}

 
void setup() {
  Serial.begin(9600);
  mbedtls_aes_context aes;
  char *key = "abcdefghijklmnop";
  unsigned char *input = (unsigned char *)"SmartComputerLabSmartComputerLab";
  unsigned char crypte[32], decrypte[32];
  
  Serial.println();  Serial.println();
  Serial.printf("%32.32s\n",input);
  encrypt(input, key, crypte, 2);
  
  for (int i = 0; i < 32; i++) { 
    char str[3];
    sprintf(str,"%02x",(int)crypte[i]);
    Serial.print(str); }
    
  Serial.println();  
  decrypt(crypte, key, decrypte,2); 
  Serial.printf("%32.32s\n",decrypte);
   
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
