#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>
#include <iostream>


using namespace std::chrono_literals;

PeerPiecesAvailability::PeerPiecesAvailability() {}

PeerPiecesAvailability::PeerPiecesAvailability(std::string bitfield) : bitfield_(bitfield) {}

bool PeerPiecesAvailability::IsPieceAvailable(size_t pieceIndex) const
{
    if (pieceIndex>bitfield_.size()){
        throw std::runtime_error("There is now piece with such index");
    }
    if (((size_t((unsigned char)bitfield_[pieceIndex/8]))&(1<<((7-(pieceIndex%8)))))>0){
        return true;
    }
    return false;
}

void PeerPiecesAvailability::SetPieceAvailability(size_t pieceIndex)
{
    if (pieceIndex>bitfield_.size()){
        throw std::runtime_error("Can not set piece availability with such index");
    }
    bitfield_[pieceIndex/8]=int((unsigned char)bitfield_[pieceIndex/8])|(1<<((7-(pieceIndex%8))));
}
size_t PeerPiecesAvailability::Size() const
{
    return (bitfield_.size()*((size_t)8));
}




PeerConnect::PeerConnect(const Peer& peer, const TorrentFile &tf, std::string selfPeerId, PieceStorage& pieceStorage) : 
                    tf_(tf), 
                    selfPeerId_(selfPeerId), 
                    socket_(peer.ip,peer.port,std::chrono::milliseconds(500),std::chrono::milliseconds(500)), 
                    terminated_(false), 
                    choked_(true),
                    pendingBlock_(false),
                    failed_(false),
                    pieceStorage_(pieceStorage) {}

void PeerConnect::Run() {
    failed_ = false;
    if (EstablishConnection()) {
        std::cout << "Connection established to peer " << socket_.GetIp() << ":" << socket_.GetPort() << std::endl;
        try{
            MainLoop();
        }catch(std::runtime_error& e){
            std::cout << e.what() << std::endl;
            failed_ = true;
            pieceStorage_.piecesInProgressCount_--;
            //pieceStorage_.mutexForQueue_.lock();
            pieceInProgress_->Reset();
            pieceStorage_.remainPieces_.push(pieceInProgress_);
            //pieceStorage_.mutexForQueue_.unlock();
        }catch(...){
            std::cout << "Something went wrong" << std::endl;
            pieceStorage_.piecesInProgressCount_--;
            failed_ = true;
        }
    } else {
        failed_ = true;
    }
}

void PeerConnect::PerformHandshake() {
    socket_.EstablishConnection();
    std::string to_send;
    to_send.push_back((char)19);
    to_send+=std::string("BitTorrent protocol");
    for (size_t i = 0; i < 8; i++)
    {
        to_send.push_back((char)0);
    }
    
    to_send+=tf_.infoHash;
    to_send+=selfPeerId_;
    socket_.SendData(to_send);
    std::string recieved = socket_.ReceiveData(68);
    if (recieved[0]!=(char)19 || recieved.substr(1,19)!="BitTorrent protocol"){
        throw std::runtime_error("handshake contains not BitTorrent protocol");
    }
    if (tf_.infoHash!=recieved.substr(28,20)){
        throw std::runtime_error("mismatched of hash");
    }
    peerId_ = recieved.substr(48,20);
}

bool PeerConnect::EstablishConnection() {
    try {
        PerformHandshake();
        ReceiveBitfield();
        SendInterested();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to establish connection with peer " << socket_.GetIp() << ":" <<
            socket_.GetPort() << " -- " << e.what() << std::endl;
        return false;
    }
}

void PeerConnect::ReceiveBitfield() {
    std::string query = socket_.ReceiveData();
    Message message = Message::Parse(query);
    if (message.id==MessageId::Unchoke){
        choked_=false;
    }else if (message.id==MessageId::BitField){
        piecesAvailability_ = PeerPiecesAvailability(query.substr(1));
    }else{
        throw std::runtime_error("Some wrong message type");
    }

}

void PeerConnect::SendInterested() {
    Message interested = Message::Init(MessageId::Interested,"");
    socket_.SendData(interested.ToString());
}

void PeerConnect::RequestPiece() {
    // if (pieceInProgress_->AllBlocksRetrieved()){
    //     pieceStorage_.PieceProcessed(pieceInProgress_);
    //     Terminate();
    //     return;
    // }
    pendingBlock_=true;
    auto block = pieceInProgress_->FirstMissingBlock();
    std::string payload;
    payload+=IntToBytes(block->piece);
    payload+=IntToBytes(block->offset);
    payload+=IntToBytes(block->length);
    Message request = Message::Init(MessageId::Request,payload);
    socket_.SendData(request.ToString());
}

void PeerConnect::Terminate() {
    std::cerr << "Terminate" << std::endl;
    terminated_ = true;
}

void PeerConnect::MainLoop() {
    std::cout << "Main loop is running" << std::endl;
    pieceInProgress_ = pieceStorage_.GetNextPieceToDownload();
    while (!terminated_) {
        std::string mes = socket_.ReceiveData();
        Message message = Message::Parse(mes);
        if (message.id == MessageId::Have){
            //std::cout << "Recieved Have" << std::endl;
            piecesAvailability_.SetPieceAvailability(BytesToInt(message.payload));
        }else if (message.id == MessageId::Piece){
            //std::cout << "Recieved Piece" << std::endl;
            pendingBlock_=false;
            size_t index = BytesToInt(message.payload.substr(0,4));
            if (index!=pieceInProgress_->GetIndex()){
                throw std::runtime_error("Received some wrong piece index");
            }
            size_t offset = BytesToInt(message.payload.substr(4,4));
            //std::cout << offset << std::endl;
            pieceInProgress_->SaveBlock(offset,message.payload.substr(8));
        }else if (message.id == MessageId::Choke){
            //std::cout << "Received Choke" << std::endl;
            choked_=true;
        }else if (message.id == MessageId::Unchoke){
            //std::cout << "Recieved Unchoke" << std::endl;
            choked_ = false;
        }else{
            throw std::runtime_error("Received some wrong message type");
        }
        if (pieceInProgress_->AllBlocksRetrieved()){
            pieceStorage_.PieceProcessed(pieceInProgress_);
            pieceInProgress_ = pieceStorage_.GetNextPieceToDownload();
            if (pieceInProgress_==nullptr){
                Terminate();
                continue;
            }
        }
        if (!choked_ && !pendingBlock_) {
            RequestPiece();
        }
    }
}

bool PeerConnect::Failed() const {
    return failed_;
}
