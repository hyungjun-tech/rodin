#pragma once
typedef unsigned char byte;

class Cryptopp
{
public:
	Cryptopp() {}
	static std::string encryptAES256(std::string inputStr);
	static std::string decryptAES256(std::string inputStr);
private:
	static char* rawKey;
	static std::string AESEncrypt(byte *key, byte *iv, std::string text, unsigned int textLen);
	static std::string AESDecrypt(byte *key, byte *iv, std::string text);
};