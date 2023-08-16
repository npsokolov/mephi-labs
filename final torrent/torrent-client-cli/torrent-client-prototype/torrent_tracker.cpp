#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>


TorrentTracker::TorrentTracker(const std::string &url) : url_(url) {}

const std::vector<Peer> &TorrentTracker::GetPeers() const {
    return peers_;
}

void TorrentTracker::UpdatePeers(const TorrentFile& tf, std::string peerId, int port) {
    peers_.clear();
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

        Bencode::Parse(res.text, Dictionary);

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
