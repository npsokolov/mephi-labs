#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory, size_t toDownload) : 
                        outputFile_(std::ofstream(outputDirectory / tf.name, std::ios_base::binary | std::ios_base::out)),
                        piecesInProgressCount_(0),
                        pieceLength_(tf.pieceLength)
{
    
    std::filesystem::resize_file(outputDirectory / tf.name,tf.length);
    size_t len = tf.length;
    size_t index = 0;
    int i = 0;
    while(len/pieceLength_>0 && i<toDownload)
    {
        i++;
        auto x = std::make_shared<Piece>(Piece(index,pieceLength_,tf.pieceHashes[index]));
        remainPieces_.push(x);
        index++;
        len-=pieceLength_;
    }
    // if (len>0){
    //     remainPieces_.emplace(std::make_shared<Piece>(index,len,tf.pieceHashes[index]));
    //     index++;
    //     len=0;
    // }
    totalPiecesCount_ = index;
}


PiecePtr PieceStorage::GetNextPieceToDownload() {
    std::lock_guard<std::mutex> guard(mutexForQueue_);
    std::cout << "1 more piece gained" << std::endl;
    if (remainPieces_.empty()){
        return nullptr;
    }
    piecesInProgressCount_++;
    auto ptr = remainPieces_.front();
    remainPieces_.pop();
    return ptr;
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    std::lock_guard<std::mutex> guard(mutexForWriting_);
    if (piece->HashMatches()){
        SavePieceToDisk(piece);
        piecesInProgressCount_--;
    }else{
        throw std::runtime_error("Received piece with wrong hash");
    }
    
}

bool PieceStorage::QueueIsEmpty() const {
    std::lock_guard<std::mutex> guard(mutexForQueue_);
    return remainPieces_.empty();
}

size_t PieceStorage::PiecesSavedToDiscCount() const{
    std::lock_guard<std::mutex> guard(mutexForWriting_);
    return piecesSavedToDiscIndices_.size();
}

size_t PieceStorage::TotalPiecesCount() const {
    return totalPiecesCount_;
}

void PieceStorage::CloseOutputFile(){
    if (outputFile_.is_open()){
        outputFile_.close();
    }
}

const std::vector<size_t>& PieceStorage::GetPiecesSavedToDiscIndices() const{
    std::lock_guard<std::mutex> guard(mutexForWriting_);
    return piecesSavedToDiscIndices_;
}

size_t PieceStorage::PiecesInProgressCount() const{
    std::lock_guard<std::mutex> guardForQueue(mutexForQueue_);
    std::lock_guard<std::mutex> guardForWriting(mutexForWriting_);
    return piecesInProgressCount_;
}

void PieceStorage::SavePieceToDisk(const PiecePtr& piece) {
    if (!outputFile_.good()){
        throw std::runtime_error("Output file is not good");
    }
    std::cout << "Downloaded piece " << piece->GetIndex() << std::endl;
    piecesSavedToDiscIndices_.emplace_back(piece->GetIndex());
    outputFile_.seekp(pieceLength_*piece->GetIndex(),std::ios::beg);
    std::string data = piece->GetData();
    outputFile_.write(data.c_str(),data.size());
}
