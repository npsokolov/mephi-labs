#include "byte_tools.h"
#include <openssl/sha.h>
#include <vector>
#include <sstream>
#include <iostream> 

size_t BytesToInt(std::string bytes) {
    size_t Bytes=0;
    for (size_t i = 0; i < 4; i++)
    {
        Bytes|=(((size_t)((unsigned char)bytes[i]))<<((size_t)8*(3-i)));
    }
    return Bytes;
}

std::string CalculateSHA1(const std::string& msg) {
    std::string SHA1Hash;

    unsigned char sha_return_statement[20];

    const unsigned char *to_encode = reinterpret_cast<const unsigned char *>(msg.c_str());
    SHA1(to_encode, msg.size(), sha_return_statement);

    for (size_t i = 0; i < 20; i++)
    {
        SHA1Hash+= sha_return_statement[i];
    }
    
    return SHA1Hash;
}

std::string IntToBytes(size_t number)
{
    std::string Bytes;
    Bytes.push_back((unsigned char)(number>>24));
    Bytes.push_back((unsigned char)((number>>16)%256));
    Bytes.push_back((unsigned char)((number>>8)%256));
    Bytes.push_back((unsigned char)(number%256));
    return Bytes;
}

std::string HexEncode(const std::string& input){
    std::stringstream ss;

    ss << std::hex << std::uppercase;

    for (int i = 0; i < input.size(); i++) {
        ss << "\\x" << (unsigned int)((unsigned char)(input[i]));
    }

    std::string hexEncode;
    ss >> hexEncode;
    return hexEncode;
}


