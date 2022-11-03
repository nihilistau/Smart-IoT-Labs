#include "mbedtls/aes.h"
 
void encrypt(char * plainText, char * key, unsigned char * outputBuffer)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb( &aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)plainText, outputBuffer);
  mbedtls_aes_free( &aes );
}
 
void decrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}

void setup() 
{
  Serial.begin(9600);while(!Serial); Serial.println();
  char * key = "abcdefghijklmnop";
  char *plainText = "SmartComputerLab";
  unsigned char cipherTextOutput[16];
  unsigned char decipheredTextOutput[16];
  
  encrypt(plainText, key, cipherTextOutput);
 
  decrypt(cipherTextOutput, key, decipheredTextOutput);
  Serial.println();
  Serial.print("Original plain text: "); Serial.println(plainText);
  Serial.println();
  Serial.println("Ciphered text:");
  for (int i = 0; i < 16; i++) 
    {
    char str[3];
    sprintf(str, "%02x", (int)cipherTextOutput[i]);
    Serial.print(str);
  }
 
  Serial.println("\n\nDeciphered text:");
  for (int i = 0; i < 16; i++) 
    { Serial.print((char)decipheredTextOutput[i]);}
}
 
void loop() {}
