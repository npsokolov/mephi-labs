#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>
#include <iostream>
#include <filesystem>

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
    }
    else
    {
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

void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary, size_t& Start_of_hash, size_t& End_of_hash)
{
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

                if (Dict_key == "info")
                {
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
                    if (data[i] == 'l')
                    { // list of lists
                        std::list<std::list<std::string>> In_list_list;
                        while (data[i] != 'e')
                        {
                            i++;
                            std::list<std::string> In_list;
                            while (data[i] != 'e')
                            {
                                std::string In_list_value = "";
                                In_list_value = GetValue(i, data);
                                In_list.push_back(In_list_value);
                            }
                            In_list_list.push_back(In_list);
                            i++;
                        }
                        i++;
                        Dictionary[Dict_key] = In_list_list;
                    }
                    else
                    { // lists of strings
                        std::list<std::string> In_list;
                        while (data[i] != 'e')
                        {
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

                if (Dict_key == "info")
                {
                    End_of_hash = i;
                }
            }
            i++;
        }
        else
        {
            std::cout << ".torrent file is not correct" << std::endl;
        }
    }
}

void Parse(std::string &data, std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> &Dictionary)
{
    size_t s = 0; // to do the same Parse without info_hash field
    size_t e = 0;
    Parse(data,Dictionary,s,e);
}


class TorrentTracker
{
public:

    TorrentTracker(const std::string &url) : url_(url) {}

    void UpdatePeers(const TorrentFile &tf, std::string peerId, int port)
    {
        std::map<std::string, std::variant<std::map<std::string, std::string>, std::string, std::list<std::string>, std::list<std::list<std::string>>>> Dictionary;
        cpr::Response res = cpr::Get(
            cpr::Url{url_},
            cpr::Parameters{
                {"info_hash", tf.infoHash},
                {"peer_id", peerId},
                {"port", std::to_string(port)},
                {"uploaded", std::to_string(0)},
                {"downloaded", std::to_string(0)},
                {"left", std::to_string(tf.length)},
                {"compact", std::to_string(1)}},
            cpr::Timeout{20000}
        );

        Parse(res.text, Dictionary);

        std::string coded_peers;
        coded_peers = std::get<1>(Dictionary["peers"]);

        const unsigned char *unsigned_coded_peers = reinterpret_cast<const unsigned char *>(coded_peers.c_str());
        for (size_t i = 0; i < coded_peers.size() / 6; i++)
        {
            std::string ip_address = "";
            unsigned int peer_port;

            peer_port = (((unsigned int)(unsigned_coded_peers[6 * i + 4])) << 8) + (unsigned int)(unsigned_coded_peers[6 * i + 5]);
            for (size_t j = 0; j < 4; j++)
            {
                ip_address += std::to_string((unsigned int)(unsigned_coded_peers[6 * i + j])) + ".";
            }
            ip_address.pop_back(); // deleting of the back dot

            Peer p;
            p.ip = ip_address;
            p.port = peer_port;
            peers_.push_back(p);
        }
    }

    const std::vector<Peer> &GetPeers() const
    {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};

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

    Parse(data, Dictionary, Start_of_hash, End_of_hash);

    auto &info = std::get<0>(Dictionary["info"]);

    TF.announce = std::get<1>(Dictionary["announce"]);
    TF.comment = std::get<1>(Dictionary["comment"]);
    auto &pieces = info["pieces"];
    for (size_t i = 0; i < pieces.size(); i += 20)
    {
        size_t length = std::min((size_t)20, pieces.size() - i);
        TF.pieceHashes.push_back(pieces.substr(i, length));
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
