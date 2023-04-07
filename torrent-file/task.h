#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>

struct TorrentFile
{
    std::string announce;                 // url трекера    udp://     This is a ‘UDP tracker protocol’.
    std::string comment;                  // Comment about the torrent file added by the creator of the torrent. See below to know how they are added at the creation of a torrent.
    std::vector<std::string> pieceHashes; // It denotes each piece’s SHA-1 hash, stored in a hash list. Its always a multiple of 160-bits as output of SHA-1 is 160-bits.
                                          // In UTF-8 encoding, it is something like this(totally unreadable).
    size_t pieceLength;                   // количество байт в каждом куске файла, на которые он поделен
                                          // степень двойки, скорее всего 2^18=256K
                                          // It denotes the length of one piece, generally 28KB.
    size_t length;                        //  Length of the file in Bytes.
    std::string name;                     // Name of the torrent file.
    std::string infoHash;                 // The SHA1-хеш для словаря с информацией.
};

size_t StringToSize_t(std::string &str)
{
    size_t answer;
    std::stringstream to_read(str);
    to_read >> answer;
    return answer;
}

std::string GetValue(size_t &start, std::string &data)
{
    std::string Value;
    std::string length_str;
    if (data[start] == 'i')
    {
        start++;
        while (data[start] != 'e')
        {
            Value += data[start++];
        }
        start++;
    }else{
        while (data[start] != ':')
        {
            length_str += data[start++];
        }
        size_t length = StringToSize_t(length_str);
        start++;
        Value = data.substr(start, length);
        start += length;
    }
    return Value;
}


TorrentFile LoadTorrentFile(const std::string &filename)
{
    std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>,std::list<std::list<std::string>>>> Dictionary;
    TorrentFile TF;
    size_t Start_of_hash=0, End_of_hash=0;
    std::ifstream File(filename);
    File.seekg(0, File.end);
    size_t File_length = File.tellg();
    std::string data(File_length, ' ');
    File.seekg(0, File.beg);
    File.read(&data[0], File_length);

    for (size_t i = 0; i < data.size(); ++i)
    {
        char key = data[i];
        if (key == 'd')
        {
            i++;
            while (data[i] != 'e')
            {
                std::string Dict_value = "", Dict_key = "";
                Dict_key = GetValue(i, data);

                if (Dict_key=="info"){
                    Start_of_hash = i;
                }

                if (data[i] == 'd') // dictionary
                {
                    i++;
                    std::map<std::string, std::string> In_dict;
                    while (data[i] != 'e')
                    {
                        std::string In_dict_value = "", In_dict_key = "";
                        In_dict_key = GetValue(i, data);
                        In_dict_value = GetValue(i, data);
                        In_dict[In_dict_key] = In_dict_value;
                    }
                    i++;
                    Dictionary[Dict_key] = In_dict;
                }
                else if (data[i] == 'l')
                {
                    i++;
                    if (data[i]=='l'){//list of lists
                        std::list<std::list<std::string>> In_list_list;
                        while (data[i]!='e'){
                            i++;
                            std::list<std::string> In_list;
                            while (data[i]!='e'){
                                std::string In_list_value = "";
                                In_list_value = GetValue(i, data);
                                In_list.push_back(In_list_value);
                            }
                            In_list_list.push_back(In_list);
                            i++;
                        }
                        i++;
                        Dictionary[Dict_key] = In_list_list;
                    }else{//lists of strings
                        std::list<std::string> In_list;
                        while (data[i]!='e'){
                            std::string In_list_value = "";
                            In_list_value = GetValue(i, data);
                            In_list.push_back(In_list_value);
                        }
                        i++;
                        Dictionary[Dict_key] = In_list;
                    }
                }
                else // string
                {
                    Dict_value = GetValue(i, data);
                    Dictionary[Dict_key] = Dict_value;
                }

                if (Dict_key=="info"){
                    End_of_hash = i;
                }
            }
            i++;
        }else{
            std::cout << ".torrent file is not correct" << std::endl;
        }
    }

    auto &info = std::get<0>(Dictionary["info"]);

    TF.announce = std::get<1>(Dictionary["announce"]);
    TF.comment = std::get<1>(Dictionary["comment"]);
    auto& pieces = info["pieces"];
    for (size_t i = 0; i < pieces.size(); i+=20)
    {
        size_t length = std::min((size_t)20,pieces.size()-i);
        TF.pieceHashes.push_back(pieces.substr(i,length));
    }
    TF.pieceLength = StringToSize_t(info["piece length"]);
    TF.length = StringToSize_t(info["length"]);
    TF.name = info["name"];

    unsigned char Sha_return_statement[20];
    std::string test = data.substr(Start_of_hash, End_of_hash - Start_of_hash);
    const unsigned char *to_encode = reinterpret_cast<const unsigned char *>(test.c_str());
    SHA1(to_encode, test.size(), Sha_return_statement);
    TF.infoHash.resize(20);
    for (size_t i = 0; i < 20; i++)
    {
        TF.infoHash[i] = Sha_return_statement[i];
    }
    return TF;
}
