#include "stdafx.h"
#include "Cryptopp.h"

#if CRYPTOPP_MSC_VERSION
# pragma warning(disable: 4505 4355)
#endif

char* Cryptopp::rawKey = "Sindoh2188!0O328So|ution158$5850";

std::string Cryptopp::AESEncrypt(byte *key, byte *iv, std::string text, unsigned int textLen)
{
	std::string ciphertext;
	std::string base64encodedciphertext;

	//
	// AES 암호화 수행
	//
	CryptoPP::AES::Encryption
		aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption
		cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter
		stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>
		(text.c_str()), textLen);
	stfEncryptor.MessageEnd();

	//
	// Base64 인코딩
	//
	CryptoPP::StringSource(ciphertext, true,
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(base64encodedciphertext), false
		) // Base64Encoder
	); // StringSource

	return base64encodedciphertext;
}

std::string Cryptopp::AESDecrypt(byte *key, byte *iv, std::string text)
{
	std::string decryptedtext;
	std::string base64decryptedciphertext;

	//
	// Base64 디코딩
	//
	CryptoPP::StringSource(text, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StringSink(base64decryptedciphertext)
		) // Base64Encoder
	); // StringSource

//
// AES 복호화
//
	CryptoPP::AES::Decryption aesDecryption(key,
		CryptoPP::AES::MAX_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption
		cbcDecryption(aesDecryption, iv);

	CryptoPP::StreamTransformationFilter
		stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
	stfDecryptor.Put(reinterpret_cast<const unsigned char*>
		(base64decryptedciphertext.c_str()), base64decryptedciphertext.size());
	stfDecryptor.MessageEnd();

	return decryptedtext;
}



std::string Cryptopp::encryptAES256(std::string inputStr)
{
	std::string encStr;

	// 키 할당
	byte key[CryptoPP::AES::MAX_KEYLENGTH];
	memset(key, 0x00, CryptoPP::AES::MAX_KEYLENGTH);

	for (unsigned int i = 0; i < CryptoPP::AES::MAX_KEYLENGTH; i++)
		key[i] = (byte)rawKey[i];
	byte iv[CryptoPP::AES::BLOCKSIZE] = { 0x08, 0x02, 0x0b, 0x00, 0x02, 0x0e, 0x01, 0x05, 0x08, 0x08, 0x0c, 0x05, 0x08, 0x05, 0x00, 0x0d };

	encStr = AESEncrypt(key, iv, inputStr, inputStr.length());
	return encStr;
}

std::string Cryptopp::decryptAES256(std::string inputStr)
{
	std::string decStr;

	// 키 할당
	byte key[CryptoPP::AES::MAX_KEYLENGTH];
	memset(key, 0x00, CryptoPP::AES::MAX_KEYLENGTH);

	for (unsigned int i = 0; i < CryptoPP::AES::MAX_KEYLENGTH; i++)
		key[i] = (byte)rawKey[i];

	byte iv[CryptoPP::AES::BLOCKSIZE] = { 0x08, 0x02, 0x0b, 0x00, 0x02, 0x0e, 0x01, 0x05, 0x08, 0x08, 0x0c, 0x05, 0x08, 0x05, 0x00, 0x0d };
	decStr = AESDecrypt(key, iv, inputStr);
	return decStr;
}

