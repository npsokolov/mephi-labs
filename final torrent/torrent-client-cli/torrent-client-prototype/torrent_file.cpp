#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>

TorrentFile LoadTorrentFile(const std::string &filename)
{
    std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> Dictionary;
    TorrentFile TF;
    size_t Start_of_hash = 0, End_of_hash = 0;
    std::ifstream File(filename);
    File.seekg(0, File.end);
    size_t File_length = File.tellg();
    std::string data(File_length, ' ');
    File.seekg(0, File.beg);
    File.read(&data[0], File_length);
    
    Bencode::Parse(data,Dictionary,Start_of_hash,End_of_hash);

    auto &info = std::get<0>(Dictionary["info"]);

    TF.announce = std::get<1>(Dictionary["announce"]);
    TF.comment = std::get<1>(Dictionary["comment"]);
    auto &pieces = info["pieces"];
    for (size_t i = 0; i < pieces.size(); i += 20)
    {
        size_t length = std::min((size_t)20, pieces.size() - i);
        TF.pieceHashes.push_back(pieces.substr(i, length));
    }
    TF.pieceLength = Bencode::StringToSize_t(info["piece length"]);
    TF.length = Bencode::StringToSize_t(info["length"]);
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
